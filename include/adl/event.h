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

typedef enum
{
  /* this type means there are no events to process */
  ADL_EVENT_NONE,

  ADL_EVENT_CLOSE,

  ADL_EVENT_KEY_DOWN,
  ADL_EVENT_KEY_UP,

  ADL_EVENT_MOUSE_MOVE,
  ADL_EVENT_MOUSE_DOWN,
  ADL_EVENT_MOUSE_UP
}
ADLEventType;

typedef struct
{
  unsigned int scancode;
}
ADLEventKeyboard;

typedef enum
{
  ADL_MOUSE_BUTTON_LEFT   = 0x01,
  ADL_MOUSE_BUTTON_MIDDLE = 0x02,
  ADL_MOUSE_BUTTON_RIGHT  = 0x04,
  ADL_MOUSE_BUTTON_SUP    = 0x08, // scoll up
  ADL_MOUSE_BUTTON_SDOWN  = 0x10  // scroll down
}
ADLMouseButton;

typedef struct
{
  int            x;
  int            y;
  ADLMouseButton buttons; // this is a bitfield
}
ADLEventMouse;

typedef struct
{
  ADLEventType type;
  ADLWindow    window;
  union
  {
    ADLEventKeyboard key;
    ADLEventMouse    mouse;
  } u;
}
ADLEvent;

#endif
