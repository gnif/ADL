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

#ifndef _H_INTERFACE_ADL
#define _H_INTERFACE_ADL

#include "adl/adl.h"
#include "adl/status.h"

#include <stdbool.h>

typedef ADL_STATUS (*ADLPfFn)();

typedef ADL_STATUS (*ADLPfProcessEvent)(ADLEvent * event);

typedef ADL_STATUS (*ADLPfWindowCreate)(const ADLWindowDef def,
    ADLWindow * result);

typedef ADL_STATUS (*ADLPfWindowFn)(ADLWindow * window);

struct ADLPlatform
{
  const char *       name;
  ADLPfFn            test;
  ADLPfFn            init;
  ADLPfFn            deinit;
  ADLPfProcessEvent  processEvent;
  ADLPfWindowCreate  windowCreate;
  ADLPfWindowFn      windowDestroy;
  ADLPfWindowFn      windowShow;
  ADLPfWindowFn      windowHide;
};

#define adl_platform(x) \
  static const void * __adl_platform_p_##x = &x;\
  static const void ** __adl_platform_##x \
  __attribute__((__used__)) \
  __attribute__((__section__("adl_platforms"))) = &__adl_platform_p_##x;

#endif
