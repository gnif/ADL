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

#ifndef _H_SRC_LINKEDLIST
#define _H_SRC_LINKEDLIST

#include "adl/status.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct _ADLLinkedListItem
{
  struct _ADLLinkedListItem * prev;
  struct _ADLLinkedListItem * next;
}
ADLLinkedListItem;

// destructor
typedef void (* ADLLinkedListItemDtor)(ADLLinkedListItem * item);

// iterator
typedef bool (* ADLLinkedListItemItor)(ADLLinkedListItem * item, void * udata);

typedef struct
{
  size_t                size;
  unsigned int          count;
  ADLLinkedListItemDtor dtor;
  ADLLinkedListItem *   head;
  ADLLinkedListItem *   tail;
}
ADLLinkedList;

ADL_STATUS adlLinkedListNew(const size_t itemSize, ADLLinkedListItemDtor dtor,
    ADLLinkedList * list);
ADL_STATUS adlLinkedListFree(ADLLinkedList * list);

ADL_STATUS adlLinkedListNewItem(ADLLinkedList * list,
    ADLLinkedListItem ** result);

ADL_STATUS adlLinkedListPush  (ADLLinkedList * list, ADLLinkedListItem *  data);
ADL_STATUS adlLinkedListPop   (ADLLinkedList * list, ADLLinkedListItem ** result);
ADL_STATUS adlLinkedListRemove(ADLLinkedList * list, ADLLinkedListItem ** item,
    bool freeItem);

void adlLinkedListIterate(ADLLinkedList * list, ADLLinkedListItemItor itor,
    void * udata);

#endif
