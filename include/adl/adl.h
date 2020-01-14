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

#include "status.h"
#include "event.h"

ADL_STATUS adlInitialize();
ADL_STATUS adlShutdown();

ADL_STATUS adlGetPlatformList(int * count, const char * names[]);
ADL_STATUS adlUsePlatform(const char * name);
ADL_STATUS adlProcessEvents(ADLEvent * event);

/**
 * Parameters for a new window
 */
typedef struct
{
  int x, y, w, h;
}
ADLWindowDef;

/**
 * Opaque window reference
 */
typedef void * ADLWindow;

ADL_STATUS adlWindowCreate(const ADLWindowDef def, ADLWindow * result);
ADL_STATUS adlWindowDestroy(ADLWindow * window);
ADL_STATUS adlWindowShow(ADLWindow window);
ADL_STATUS adlWindowHide(ADLWindow window);

#endif
