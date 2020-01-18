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

#ifndef _H_ADL_XCB
#define _H_ADL_XCB

#include "adl/adl.h"
#include "src/image.h"
#include "src/window.h"

#include <xcb/xcb.h>
#include <xcb/xkb.h>

struct State
{
  xcb_connection_t * xcb;
  int                fd;
  xcb_screen_t *     screen;
  char               keyMap[256][5];
};

extern struct State this;

typedef struct
{
  xcb_window_t window;
  xcb_window_t parent;
  uint16_t     eventMask;
  int          transX, transY;
  bool         grabbed, relative;

  // window position & size
  int x, y, w, h;

  // pointer state information
  int            pointerX, pointerY;
  ADLMouseButton mouseButtonState;

  // pointer warp information
  bool              warping;
  xcb_void_cookie_t warpCookie;
  int               warpX, warpY;
}
WindowData;

#endif
