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

#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/render.h>

struct State
{
  Display *          display;
  xcb_connection_t * xcb;
  int                fd;
  xcb_screen_t *     screen;
  char               keyMap[256][5];

  xcb_render_pictforminfo_t formatRGB, formatRGBA, formatARGB;
  xcb_render_pictforminfo_t formatBGR, formatBGRA, formatABGR;

  xcb_cursor_context_t * cursorContext;
  xcb_cursor_t defaultPointer;
  xcb_pixmap_t blankPixmap;
  xcb_cursor_t blankPointer;
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
  int bpp;

  // pointer state information
  xcb_cursor_t   currentPointer;
  int            pointerX, pointerY;
  ADLMouseButton mouseButtonState;

  // pointer warp information
  bool              warping;
  xcb_void_cookie_t warpCookie;
  int               warpX, warpY;
}
WindowData;

#endif
