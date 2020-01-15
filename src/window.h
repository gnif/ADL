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

#ifndef _H_SRC_WINDOW
#define _H_SRC_WINDOW

#include "linkedlist.h"
#include "adl/window.h"

#include <stdint.h>

typedef struct
{
  ADLLinkedListItem item;
  ADLWindow window;
}
ADLWindowListItem;

typedef uint64_t ADLWindowId;

#define ADL_WINDOW_LIST_ITEM_SIZE (sizeof(ADLWindowListItem) + sizeof(ADLWindowId))

#define ADL_SET_WINDOW_ID(x, v) \
  *(ADLWindowId *)(ADL_CHECK_TYPE(ADLWindow *, x) + 1) = v

#define ADL_GET_WINDOW_ID(x) \
  (*(ADLWindowId *)(ADL_CHECK_TYPE(ADLWindow *, x) + 1))

#define ADL_GET_WINDOW_DATA(x) \
  ((void *)((ADLWindowId *)(ADL_CHECK_TYPE(ADLWindow *, x) + 1) + 1))

void windowListItemDestructor(void * item);
ADLWindow * windowFindById(ADLWindowId id);

#endif
