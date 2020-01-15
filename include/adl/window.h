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

#ifndef _H_ADL_WINDOW
#define _H_ADL_WINDOW

#include "adl/status.h"

#include <stdbool.h>

typedef enum
{
  ADL_WINDOW_FLAG_VCETNER    = 0x01,
  ADL_WINDOW_FLAG_HCENTER    = 0x02,
  ADL_WINDOW_FLAG_BORDERLESS = 0x04,
  ADL_WINDOW_FLAG_FULLSCREEN = 0x08
}
ADLWindowFlag;

/**
 * Parameters for a new window
 */
typedef struct
{
  int           x, y;
  int           w, h;
  ADLWindowFlag flags;
  const char *  title;
}
ADLWindowDef;

typedef struct
{
  int          x, y;
  unsigned int w, h;
  bool         visible;

  bool   haveMousePos;
  int    mouseX, mouseY;
}
ADLWindow;

ADL_STATUS adlWindowCreate(const ADLWindowDef def, ADLWindow ** result);
ADL_STATUS adlWindowDestroy(ADLWindow ** window);
ADL_STATUS adlWindowShow(ADLWindow * window);
ADL_STATUS adlWindowHide(ADLWindow * window);
ADL_STATUS adlWindowSetTitle(ADLWindow * window, const char * title);

#endif
