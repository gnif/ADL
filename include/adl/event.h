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

#ifndef _H_ADL_EVENT
#define _H_ADL_EVENT

#include "adl.h"
#include "window.h"

#include <stdint.h>

typedef enum
{
  /* this type means there are no events to process */
  ADL_EVENT_NONE,

  ADL_EVENT_QUIT,
  ADL_EVENT_CLOSE,
  ADL_EVENT_PAINT,
  ADL_EVENT_SHOW,
  ADL_EVENT_HIDE,
  ADL_EVENT_WINDOW_CHANGE,

  ADL_EVENT_KEY_DOWN,
  ADL_EVENT_KEY_UP,

  ADL_EVENT_MOUSE_MOVE,
  ADL_EVENT_MOUSE_DOWN,
  ADL_EVENT_MOUSE_UP,
  ADL_EVENT_MOUSE_ENTER,
  ADL_EVENT_MOUSE_LEAVE
}
ADLEventType;

typedef struct
{
  int x, y, w, h;
}
ADLEventWindow;

typedef struct
{
  int  x, y, w, h;
  bool more;
}
ADLEventPaint;

typedef struct
{
  const char * keyname;
  uint8_t      scancode;
}
ADLEventKeyboard;

typedef enum
{
  ADL_MOUSE_BUTTON_LEFT    = 0x001,
  ADL_MOUSE_BUTTON_MIDDLE  = 0x002,
  ADL_MOUSE_BUTTON_RIGHT   = 0x004,
  ADL_MOUSE_BUTTON_BACK    = 0x008,
  ADL_MOUSE_BUTTON_FORWARD = 0x010,

  // wheel events
  ADL_MOUSE_BUTTON_WUP     = 0x020,
  ADL_MOUSE_BUTTON_WDOWN   = 0x040,
  ADL_MOUSE_BUTTON_WLEFT   = 0x080,
  ADL_MOUSE_BUTTON_WRIGHT  = 0x100,

  // unknown/custom buttons have this bit set
  ADL_MOUSE_BUTTON_CUSTOM  = 0x10000
}
ADLMouseButton;

typedef struct
{
  // current mouse position and relative offset since last event
  int  x    , y;
  int  relX , relY;

  // true if there is a mouse warp pending
  bool warping;
  // true if the mouse was warped (relative mode)
  bool warp;
  // how far it was warped
  int  warpX, warpY;

  // bitfield of the held/down mouse buttons
  ADLMouseButton buttons;
}
ADLEventMouse;

typedef struct
{
  ADLEventType type;
  ADLWindow *  window;
  union
  {
    ADLEventWindow   win;
    ADLEventPaint    paint;
    ADLEventKeyboard key;
    ADLEventMouse    mouse;
  } u;
}
ADLEvent;

#endif
