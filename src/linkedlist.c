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
#include "linkedlist.h"

#include <stdlib.h>
#include <assert.h>

ADL_STATUS adlLinkedListNew(const size_t itemSize, ADLLinkedListItemDtor dtor,
    ADLLinkedList * list)
{
  if (!list)
  {
    ADL_BUG(ADL_ERR_INVALID_ARGUMENT, "list == NULL");
    return ADL_ERR_INVALID_ARGUMENT;
  }

  if (itemSize < sizeof(ADLLinkedListItem))
  {
    ADL_BUG(ADL_ERR_INVALID_ARGUMENT, "itemSize < sizeof(ADLLinkedListItem)");
    return ADL_ERR_INVALID_ARGUMENT;
  }

  if (!dtor)
    dtor = (void *)(ADLLinkedListItem *)free;

  list->size  = itemSize;
  list->count = 0;
  list->dtor  = dtor;
  list->head  = NULL;
  list->tail  = NULL;

  return ADL_OK;
}

ADL_STATUS adlLinkedListFree(ADLLinkedList * list)
{
  if (!list)
  {
    ADL_BUG(ADL_ERR_INVALID_ARGUMENT, "list == NULL");
    return ADL_ERR_INVALID_ARGUMENT;
  }

  ADLLinkedListItem * next;
  for(ADLLinkedListItem * item = list->head; item != NULL; item = next)
  {
    next = item->next;
    list->dtor(item);
  }

  list->size  = 0;
  list->count = 0;
  list->head  = NULL;
  list->tail  = NULL;

  return ADL_OK;
}

ADL_STATUS adlLinkedListNewItem(ADLLinkedList * list,
    ADLLinkedListItem ** result)
{
  assert(list);
  assert(result);
  assert(list->size > 0);

  *result = NULL;
  ADLLinkedListItem * item = calloc(1, list->size);
  if (!item)
  {
    ADL_ERROR(ADL_ERR_NO_MEM, "unable to allocate %lu bytes", list->size);
    return ADL_ERR_NO_MEM;
  }

  adlLinkedListPush(list, item);
  *result = item;

  return ADL_OK;
}

ADL_STATUS adlLinkedListPush(ADLLinkedList * list, ADLLinkedListItem * data)
{
  assert(list);
  assert(list->size > 0);

  ADLLinkedListItem * item = (ADLLinkedListItem *)data;

  if (!list->head)
    list->head = item;

  if (list->tail)
    list->tail->next = item;

  item->prev = list->tail;
  item->next = NULL;
  list->tail = item;
  ++list->count;

  return ADL_OK;
}

ADL_STATUS adlLinkedListPop(ADLLinkedList * list, ADLLinkedListItem ** result)
{
  assert(list);
  assert(list->size > 0);

  if (result)
    *result = NULL;

  if (!list->tail)
    return ADL_ERR_EMPTY;

  ADLLinkedListItem * item = list->tail;
  list->tail = list->tail->prev;
  if (!list->tail)
    list->head = NULL;

  --list->count;

  if (result)
    *result = item;
  else
    list->dtor(item);

  return ADL_OK;
}

ADL_STATUS adlLinkedListRemove(ADLLinkedList * list, ADLLinkedListItem ** item,
    bool freeItem)
{
  assert(list);
  assert(list->size > 0);
  assert(item && *item);

  ADLLinkedListItem * i = *item;
  if (i->prev)
    i->prev->next = i->next;

  if (list->head == i)
    list->head = i->next;

  if (list->tail == i)
    list->tail = i->next;

  --list->count;

  if (freeItem)
  {
    list->dtor(i);
    *item = NULL;
  }

  return ADL_OK;
}

void adlLinkedListIterate(ADLLinkedList * list, ADLLinkedListItemItor itor,
    void * udata)
{
  assert(list);

  ADLLinkedListItem * next;
  for(ADLLinkedListItem * item = list->head; item != NULL; item = next)
  {
    next = item->next;
    if (!itor(item, udata))
      break;
  }
}
