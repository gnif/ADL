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

#include "adl/thread.h"
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>

static void * threadFn(void * opaque)
{
  ADLThread * thread = (ADLThread *)opaque;
  return thread->function(thread, opaque);
}

ADL_STATUS adlThreadCreate(ADLThreadFn fn, void * udata, ADLThread * result)
{
  result->function = fn;
  result->udata    = udata;
  atomic_store(&result->running, true);
  if (pthread_create(&result->thread, NULL, threadFn, result) != 0)
    return ADL_ERR_PLATFORM;
  return ADL_OK;
}

ADL_STATUS adlThreadJoin(ADLThread * thread, void ** result, int timeout)
{
  int ret;

  if (timeout < 0)
    ret = pthread_join(thread->thread, result);
  else if (timeout == 0)
  {
    ret = pthread_tryjoin_np(thread->thread, result);
    if (ret == EBUSY)
      return ADL_ERR_BUSY;
  }
  else
  {
    const struct timespec ts =
    {
      .tv_sec  = timeout / 1000,
      .tv_nsec = ((unsigned long)timeout % 1000) * 1000000
    };
    ret = pthread_timedjoin_np(thread->thread, result, &ts);
    if (ret == ETIMEDOUT)
      return ADL_ERR_TIMEOUT;
  }

  if (ret != 0)
    return ADL_ERR_PLATFORM;

  return ADL_OK;
}
