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

#include "image.h"
#include "adl.h"
#include "window.h"

#include <stdlib.h>

void imageListItemDestructor(ADLLinkedListItem  * item)
{
  ADLImageListItem * ii = (ADLImageListItem*)item;

  ADL_STATUS status;
  if ((status = adl.platform->imageDestroy(&ii->image)) != ADL_OK)
    ADL_ERROR(status, "imageDestroy failed");

  free(ii);
}

ADLImage * imageFindById(ADLWindow * window, ADLImageId id)
{
  ADLLinkedListItem * item;
  for(item = ADL_GET_WINDOW_IMAGE_LIST(window)->head;
      item != NULL; item = item->next)
  {
    ADLImageListItem * li = (ADLImageListItem *)item;
    if (ADL_GET_IMAGE_ID(&li->image) == id)
      return &li->image;
  }
  return NULL;
}

/* get the backends supported by the platform */
ADL_STATUS adlImageGetSupported(const ADLImageBackend ** backends)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(backends);
  return adl.platform->imageGetSupported(backends);
}

/* create a new image per the definition */
ADL_STATUS adlImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage ** result)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  ADL_NOT_NULL_CHECK(result);

  ADL_STATUS status;

  *result = NULL;
  ADLImageListItem * item;
  status = adlLinkedListNewItem(ADL_GET_WINDOW_IMAGE_LIST(window),
      (ADLLinkedListItem **)&item);
  if (status != ADL_OK)
    return status;

  ADLImage * img = &item->image;
  img->window = window;

  status = adl.platform->imageCreate(window, def, img);
  if (status == ADL_OK && !item->id)
  {
    ADL_BUG(ADL_ERR_PLATFORM,
        "%s->imageCreate did not set the image id", adl.platform->name);
    adlLinkedListPop(&adl.windowList, NULL);
    return ADL_ERR_PLATFORM;
  }

  *result = img;
  return status;
}

/* destroy a image */
ADL_STATUS adlImageDestroy(ADLImage ** image)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(image);

  if (!*image)
    return ADL_OK;

  ADL_STATUS status = ADL_OK;

  ADLLinkedList    * ll = ADL_GET_IMAGE_LIST(*image);
  ADLImageListItem * li = ADL_IMAGE_GET_LIST_ITEM(*image);
  adlLinkedListRemove(ll, (ADLLinkedListItem **)&li, true);

  *image = NULL;
  return status;
}

/* update the image from it's backend storage */
ADL_STATUS adlImageUpdate(ADLImage * image)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(image);
  return adl.platform->imageUpdate(image);
}
