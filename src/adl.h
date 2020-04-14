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

#ifndef _H_SRC_ADL
#define _H_SRC_ADL

#include "window.h"
#include "interface/adl.h"
#include "linkedlist.h"

#include <stdbool.h>
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ENDIAN_LITTLE
#else
#define ENDIAN_BIG
#endif

struct ADL
{
  bool initDone;
  uint64_t startTime;

  const struct ADLPlatform ** platformList;
  int                         platformListCount;
  int                         numPlatforms;

  const struct ADLPlatform * platform;

  ADLLinkedList windowList;
};

extern struct ADL adl;

#define ADL_INITCHECK \
  if (!adl.initDone) \
  { \
    ADL_ERROR(ADL_ERR_NOT_INITIALIZED, "not initialized"); \
    return ADL_ERR_NOT_INITIALIZED; \
  }

#define ADL_NOT_NULL_CHECK(x) \
  if (!x) \
  { \
    ADL_ERROR(ADL_ERR_INVALID_ARGUMENT, #x " == NULL"); \
    return ADL_ERR_INVALID_ARGUMENT; \
  }

#define ADL_CHECK_TYPE(type, x) \
  ({ \
    type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    (x); \
  })

#endif
