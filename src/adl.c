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
#include "window.h"
#include "linkedlist.h"

#include "interface/adl.h"

#include "adl/adl.h"
#include "adl/status.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

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
    ADL_ERROR(ADL_ERR_ALREADY_INITIALIZED, "already initialized");
    return ADL_ERR_ALREADY_INITIALIZED;
  }

  adl.startTime = adlGetClockMS();

  /* build an array of the platform members for validation */
  struct CheckField {
    const char * name;
    uintptr_t    offset;
  };
  #define ADL_FIELD(_type, _name) { \
    .name   = #_name, \
    .offset = offsetof(struct ADLPlatform, _name) \
  },
  static const struct CheckField checkFields[] =
  {
    ADL_PLATFORM_FIELDS
    { 0 }
  };
  #undef ADL_FIELD

  /* call the platforms test methods, removing any that fail or are
   * unsupported to prevent future evaluation */
  for(int i = 0; i < adl.platformListCount; ++i)
  {

    const struct ADLPlatform * p = adl.platformList[i];

    /* perform a sanity check on the platform members */
    bool error = false;
    for(const struct CheckField *f = checkFields; f->name; ++f)
    {
      const void ** check = (const void **)((const uint8_t*)p + f->offset);
      if (!*check)
      {
        ADL_BUG(ADL_ERR_PLATFORM, "%s: `%s` is NULL", p->name, f->name);
        error = true;
      }
    }

    if (error)
    {
      ADL_BUG(ADL_ERR_PLATFORM,
          "%s: Implemention is incomplete and has been disabled", p->name);
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
    ADL_WARN(status,
        "Platform `%s` failed with the error: %s",
        p->name,
        adlStatusString(status));
    adl.platformList[i] = NULL;
  }

  ADL_INFO(status,
      "%d platform(s) available", adl.numPlatforms);

  if (adl.numPlatforms == 0)
  {
    status = ADL_ERR_UNSUPPORTED;
    ADL_ERROR(status,
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

ADL_STATUS adlQuit()
{
  ADL_INITCHECK;

  ADLEvent event = {
    .type = ADL_EVENT_QUIT
  };

  adlLinkedListIterate(&adl.windowList, windowListItemEvent, &event);
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
    ADL_ERROR(ADL_ERR_INVALID_PLATFORM, "The platform `%s` is unknown", name);
    return ADL_ERR_INVALID_PLATFORM;
  }

  ADL_STATUS status;
  if ((status = adlLinkedListNew(
          sizeof(ADLWindowListItem) + adl.platform->windowDataSize,
          windowListItemDestructor, &adl.windowList)) != ADL_OK)
    return status;

  if ((status = adl.platform->init()) != ADL_OK)
  {
    ADL_ERROR(status, "Platform `%s` initialization failed", name);
    return status;
  }

  ADL_INFO(ADL_OK, "Using platform: %s", name);
  return ADL_OK;
}

ADL_STATUS adlProcessEvent(int timeout, ADLEvent * event)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(event);
  ADL_STATUS status;

  memset(event, 0, sizeof(ADLEvent));

  if ((status = adl.platform->processEvent(timeout, event)) != ADL_OK)
    return status;

  if (event->type == ADL_EVENT_NONE)
    return status;

  ADLWindow * window = event->window;
  switch(event->type)
  {
    case ADL_EVENT_SHOW:
      if (!window)
        break;

      if (window->visible)
      {
        // swallow repeat events
        event->type = ADL_EVENT_NONE;
        break;
      }
      window->visible = true;
      break;

    case ADL_EVENT_HIDE:
      if (!window)
        break;

      if (!window->visible)
      {
        // swallow repeat events
        event->type = ADL_EVENT_NONE;
        break;
      }
      window->visible = false;
      break;

    case ADL_EVENT_MOUSE_MOVE :
    case ADL_EVENT_MOUSE_DOWN :
    case ADL_EVENT_MOUSE_UP   :
    case ADL_EVENT_MOUSE_ENTER:
    case ADL_EVENT_MOUSE_LEAVE:
      if (!window)
        break;

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

      // if the mouse was warped null it out
      if (event->u.mouse.warp)
      {
        event->u.mouse.relX -= event->u.mouse.warpX;
        event->u.mouse.relY -= event->u.mouse.warpY;
      }

      // if there has been no change, swallow the event
      if (event->type == ADL_EVENT_MOUSE_MOVE &&
          event->u.mouse.relX    == 0 &&
          event->u.mouse.relY    == 0 &&
          event->u.mouse.x       == window->mouseX &&
          event->u.mouse.y       == window->mouseY &&
          event->u.mouse.warping == window->mouseWarping &&
          event->u.mouse.warp    == window->mouseWarp)
        event->type = ADL_EVENT_NONE;

      window->mouseX       = event->u.mouse.x;
      window->mouseY       = event->u.mouse.y;
      window->mouseWarping = event->u.mouse.warping;
      window->mouseWarp    = event->u.mouse.warp;
      break;

    case ADL_EVENT_WINDOW_CHANGE:
      if (!window)
        break;

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

ADL_STATUS adlFlush()
{
  ADL_INITCHECK;
  return adl.platform->flush();
}

ADL_STATUS adlPointerWarp(ADLWindow * window, int x, int y)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  return adl.platform->pointerWarp(window, x, y);
}

ADL_STATUS adlPointerVisible(ADLWindow * window, bool visible)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  return adl.platform->pointerVisible(window, visible);
}

ADL_STATUS adlPointerSetCursor(ADLWindow * window, ADLImage * source,
    ADLImage * mask, int x, int y)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  return adl.platform->pointerSetCursor(window, source, mask, x, y);
}

#if defined(ADL_HAS_EGL)
ADL_STATUS adlEGLGetDisplay(EGLDisplay ** display)
{
  ADL_INITCHECK;
  return adl.platform->eglGetDisplay(display);
}

ADL_STATUS adlEGLCreateWindowSurface(EGLDisplay * display, EGLint * config,
  ADLWindow * window, const EGLint * attribs, EGLSurface * surface)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  return adl.platform->eglCreateWindowSurface(
    display, config, window, attribs, surface);
}
#endif
