/*
  MIT License

  Copyright (c) 2020 Geoffrey McRae <geoff@hostfission.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "adl.h"
#include "logging.h"
#include "linkedlist.h"

#include "interface/adl.h"

#include "adl/adl.h"
#include "adl/status.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

struct ADL
{
  bool initDone;

  const struct ADLPlatform ** platformList;
  int                         platformListCount;
  int                         numPlatforms;

  const struct ADLPlatform * platform;

  ADLLinkedList windowList;
};

typedef struct
{
  ADLLinkedListItem item;
  ADLWindow window;
}
ADLWindowListItem;

struct ADL adl = { 0 };

ADL_STATUS adlInitialize()
{
  ADL_STATUS status = ADL_OK;

  /* get the platform list and count */
  extern const void * __start_adl_platforms;
  extern const void * __stop_adl_platforms;

  adl.platformListCount = ((uintptr_t)&__stop_adl_platforms -
    (uintptr_t)&__start_adl_platforms) / sizeof(uintptr_t);
  adl.platformList      =
    (const struct ADLPlatform **)__start_adl_platforms;

  if (adl.initDone)
  {
    DEBUG_ERROR(ADL_ERR_ALREADY_INITIALIZED, "already initialized");
    return ADL_ERR_ALREADY_INITIALIZED;
  }

  /* call the platforms test methods, removing any that fail or are
   * unsupported to prevent future evaluation */
  for(int i = 0; i < adl.platformListCount; ++i)
  {
    const struct ADLPlatform * p = adl.platformList[i];

    /* perform a sanity check on the struct */
    if (
      !p->test          ||
      !p->init          ||
      !p->deinit        ||
      !p->processEvent  ||
      !p->windowCreate  ||
      !p->windowDestroy ||
      !p->windowShow    ||
      !p->windowHide    ||
      0
    ) {
      DEBUG_BUG(ADL_ERR_PLATFORM,
          "Platform `%s` is incomplete", p->name);
      adl.platformList[i] = NULL;
      continue;
    }

    if ((status = p->test()) == ADL_OK)
    {
      ++adl.numPlatforms;
      continue;
    }

    /* not an error, the platform is just not supported */
    if (status == ADL_ERR_UNSUPPORTED)
    {
      adl.platformList[i] = NULL;
      continue;
    }

    /* treat everything else as a warning */
    DEBUG_WARN(status,
        "Platform `%s` failed with the error: %s",
        p->name,
        adlStatusString(status));
    adl.platformList[i] = NULL;
  }

  DEBUG_INFO(status,
      "%d platform(s) available", adl.numPlatforms);

  if (adl.numPlatforms == 0)
  {
    status = ADL_ERR_UNSUPPORTED;
    DEBUG_ERROR(status,
      "Unable to start, no supported platforms found");
  }

  adl.initDone = true;
  return status;
}

ADL_STATUS adlShutdown()
{
  ADL_INITCHECK;

  adlLinkedListFree(&adl.windowList);
  return ADL_OK;
}

ADL_STATUS adlGetPlatformList(int * count, const char * names[])
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(count);

  *count = adl.numPlatforms;
  if (!names)
    return ADL_OK;

  for(int i = 0; i < adl.platformListCount; ++i)
  {
    if (!adl.platformList[i])
      continue;
    *names = adl.platformList[i]->name;
    ++names;
  }

  return ADL_OK;
}


ADL_STATUS adlUsePlatform(const char * name)
{
  ADL_INITCHECK;

  for(int i = 0; i < adl.platformListCount; ++i)
  {
    if (!adl.platformList[i])
      continue;

    if (strcmp(adl.platformList[i]->name, name) == 0)
    {
      adl.platform = adl.platformList[i];
      break;
    }
  }

  if (!adl.platform)
  {
    DEBUG_ERROR(ADL_ERR_INVALID_PLATFORM, "The platform `%s` is unknown", name);
    return ADL_ERR_INVALID_PLATFORM;
  }

  ADL_STATUS status;
  if ((status = adlLinkedListNew(sizeof(ADLWindowListItem) + sizeof(void *),
          &adl.windowList)) != ADL_OK)
    return status;

  if ((status = adl.platform->init()) != ADL_OK)
  {
    DEBUG_ERROR(status, "Platform `%s` initialization failed", name);
    return status;
  }

  DEBUG_INFO(ADL_OK, "Using platform: %s", name);
  return ADL_OK;
}

ADL_STATUS adlWindowCreate(const ADLWindowDef def, ADLWindow ** result)
{
  ADL_INITCHECK;
  ADL_STATUS status;

  if (!result)
  {
    DEBUG_ERROR(ADL_ERR_INVALID_ARGUMENT, "result == NULL");
    return ADL_ERR_INVALID_ARGUMENT;
  }

  *result = NULL;

  ADLWindowListItem * item;
  status = adlLinkedListNewItem(&adl.windowList, (ADLLinkedListItem **)&item);
  if (status != ADL_OK)
    return status;

  ADLWindow * win = &item->window;

  win->x = def.x;
  win->y = def.y;
  win->w = def.w;
  win->h = def.h;

  status = adl.platform->windowCreate(def, win);
  if (status == ADL_OK && (!ADL_GET_WINDOW_DATA(win)))
  {
    DEBUG_BUG(ADL_ERR_PLATFORM,
        "%s->windowCreate did not set the window data", adl.platform->name);

    adlLinkedListPop(&adl.windowList, NULL);
    return ADL_ERR_PLATFORM;
  }

  *result = win;
  return status;
}

ADL_STATUS adlWindowDestroy(ADLWindow ** window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  if (!*window)
    return ADL_OK;

  ADL_STATUS status;
  if ((status = adl.platform->windowDestroy(*window)) != ADL_OK)
  {
    DEBUG_ERROR(status, "windowDestroy failed");
    return status;
  }

  ADLLinkedListItem * item;
  for(item = adl.windowList.head; item != NULL; item = item->next)
  {
    ADLWindowListItem * li = (ADLWindowListItem *)item;
    if (&li->window != *window)
      continue;

    if ((status = adlLinkedListRemove(&adl.windowList, &item, true)) != ADL_OK)
      DEBUG_BUG(status, "failed to remove window from the windowList");
    break;
  }

  *window = NULL;
  return status;
}

ADL_STATUS adlWindowShow(ADLWindow * window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowShow(window);
}

ADL_STATUS adlWindowHide(ADLWindow * window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowHide(window);
}

ADL_STATUS adlProcessEvent(ADLEvent * event)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(event);
  ADL_STATUS status;

  event->type   = ADL_EVENT_NONE;
  event->window = NULL;

  void * windowData = NULL;
  if ((status = adl.platform->processEvent(event, &windowData)) != ADL_OK)
    return status;

  if (event->type == ADL_EVENT_NONE)
    return status;

  ADLWindow * window = NULL;
  if (windowData)
  {
    ADLLinkedListItem * item;
    for(item = adl.windowList.head; item != NULL; item = item->next)
    {
      ADLWindowListItem * li = (ADLWindowListItem *)item;
      if (ADL_GET_WINDOW_DATA(&li->window) != windowData)
        continue;

      event->window = &li->window;
      window        = &li->window;
      break;
    }
  }

  switch(event->type)
  {
    case ADL_EVENT_SHOW:
      if (window->visible)
      {
        // swallow repeat events
        event->type = ADL_EVENT_NONE;
        break;
      }
      window->visible = true;
      break;

    case ADL_EVENT_HIDE:
      if (!window->visible)
      {
        // swallow repeat events
        event->type = ADL_EVENT_NONE;
        break;
      }
      window->visible = false;
      break;

    case ADL_EVENT_MOUSE_MOVE:
    case ADL_EVENT_MOUSE_DOWN:
    case ADL_EVENT_MOUSE_UP  :
      // fill in the relX and relY fields
      if (!window->haveMousePos)
      {
        window->haveMousePos = true;
        window->mouseX       = event->u.mouse.x;
        window->mouseY       = event->u.mouse.y;
        event->u.mouse.relX  = 0;
        event->u.mouse.relY  = 0;
        break;
      }

      event->u.mouse.relX = event->u.mouse.x - window->mouseX;
      event->u.mouse.relY = event->u.mouse.y - window->mouseY;
      window->mouseX      = event->u.mouse.x;
      window->mouseY      = event->u.mouse.y;
      break;

    case ADL_EVENT_WINDOW_CHANGE:
      if (!(window->x != event->u.win.x ||
            window->y != event->u.win.y ||
            window->w != event->u.win.w ||
            window->h != event->u.win.h))
      {
        event->type = ADL_EVENT_NONE;
        break;
      }

      window->x = event->u.win.x;
      window->y = event->u.win.y;
      window->w = event->u.win.w;
      window->h = event->u.win.h;
      break;

    default:
      break;
  }

  return status;
}
