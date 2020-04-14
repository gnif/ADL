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

#ifndef _H_ADL
#define _H_ADL

#include "logging.h"
#include "status.h"
#include "window.h"
#include "event.h"
#include "image.h"
#include "util.h"
#include "thread.h"
#include "timer.h"

#if defined(ADL_HAS_EGL)
#include <EGL/egl.h>
#endif

ADL_STATUS adlInitialize();
ADL_STATUS adlShutdown();
ADL_STATUS adlQuit();

ADL_STATUS adlGetPlatformList(int * count, const char * names[]);
ADL_STATUS adlUsePlatform(const char * name);
ADL_STATUS adlProcessEvent(int timeout, ADLEvent * event);
ADL_STATUS adlFlush(void);

ADL_STATUS adlPointerWarp(ADLWindow * window, int x, int y);
ADL_STATUS adlPointerVisible(ADLWindow * window, bool visible);
ADL_STATUS adlPointerSetCursor(ADLWindow * window, ADLImage * source,
    ADLImage * mask, int x, int y);

#if defined(ADL_HAS_EGL)
ADL_STATUS adlEGLGetDisplay(EGLDisplay ** display);
ADL_STATUS adlEGLCreateWindowSurface(EGLDisplay * display, EGLint * config,
  ADLWindow * window, const EGLint * attribs, EGLSurface * surface);
#endif

#endif
