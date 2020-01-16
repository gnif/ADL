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

#include "window.h"
#include "src/window.h"
#include "src/adl.h"
#include "src/logging.h"
#include "src/image.h"

#include <stdlib.h>

void windowListItemDestructor(void * item)
{
  ADLWindow * window = (ADLWindow *)item;

  ADL_STATUS status;
  if ((status = adl.platform->windowDestroy(window)) != ADL_OK)
    DEBUG_ERROR(status, "windowDestroy failed");

  free(window);
}

ADLWindow * windowFindById(ADLWindowId id)
{
  ADLLinkedListItem * item;
  for(item = adl.windowList.head; item != NULL; item = item->next)
  {
    ADLWindowListItem * li = (ADLWindowListItem *)item;
    if (ADL_GET_WINDOW_ID(&li->window) == id)
      return &li->window;
  }
  return NULL;
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

  /* initialize the image list */
  if ((status = adlLinkedListNew(
          ADL_IMAGE_LIST_ITEM_SIZE + adl.platform->imageDataSize,
          imageListItemDestructor, &item->imageList)) != ADL_OK)
  {
    adlLinkedListPop(&adl.windowList, NULL);
    return status;
  }

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

  ADL_STATUS status = ADL_OK;
  adlLinkedListFree(ADL_GET_WINDOW_IMAGE_LIST(*window));

  ADLLinkedListItem * item;
  for(item = adl.windowList.head; item != NULL; item = item->next)
  {
    ADLWindowListItem * li = (ADLWindowListItem *)item;
    if (&li->window != *window)
      continue;

    // platform destroy is called by the destructor
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

ADL_STATUS adlWindowSetClassName(ADLWindow * window, const char * className)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowSetClassName(window, className);
}

ADL_STATUS adlWindowSetTitle(ADLWindow * window, const char * title)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowSetTitle(window, title);
}

ADL_STATUS adlWindowSetGrab(ADLWindow * window, bool enable)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowSetGrab(window, enable);
}

ADL_STATUS adlWindowSetRelative(ADLWindow * window, bool enable)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowSetRelative(window, enable);
}
