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
  ADL_WINDOW_FLAG_CENTER       = 0x0001,
  ADL_WINDOW_FLAG_MODAL        = 0x0002,
  ADL_WINDOW_FLAG_STICKY       = 0x0004,
  ADL_WINDOW_FLAG_MAXV         = 0x0008,
  ADL_WINDOW_FLAG_MAXH         = 0x0010,
  ADL_WINDOW_FLAG_SHADED       = 0x0020,
  ADL_WINDOW_FLAG_SKIP_TASKBAR = 0x0040,
  ADL_WINDOW_FLAG_SKIP_PAGER   = 0x0080,
  ADL_WINDOW_FLAG_HIDDEN       = 0x0100,
  ADL_WINDOW_FLAG_FULLSCREEN   = 0x0200,
  ADL_WINDOW_FLAG_ABOVE        = 0x0400,
  ADL_WINDOW_FLAG_BELOW        = 0x0800,
  ADL_WINDOW_FLAG_ATTENTION    = 0x1000,
  ADL_WINDOW_FLAG_FOCUSED      = 0x2000
}
ADLWindowFlag;

typedef enum
{
  ADL_WINDOW_TYPE_DESKTOP,
  ADL_WINDOW_TYPE_DOCK,
  ADL_WINDOW_TYPE_TOOLBAR,
  ADL_WINDOW_TYPE_MENU,
  ADL_WINDOW_TYPE_UTILITY,
  ADL_WINDOW_TYPE_SPLASH,
  ADL_WINDOW_TYPE_DIALOG,
  ADL_WINDOW_TYPE_DROPDOWN,
  ADL_WINDOW_TYPE_POPUP,
  ADL_WINDOW_TYPE_TOOLTIP,
  ADL_WINDOW_TYPE_NOTIFICATION,
  ADL_WINDOW_TYPE_COMBO,
  ADL_WINDOW_TYPE_DND,
  ADL_WINDOW_TYPE_NORMAL
}
ADLWindowType;

typedef struct _ADLWindow ADLWindow;

/**
 * Parameters for a new window
 */
typedef struct
{
  ADLWindow *   parent; // null if a top level window
  const char *  title;
  const char *  className;
  ADLWindowType type;
  ADLWindowFlag flags; // this is a bitfield
  bool          borderless;
  int           x, y;
  int           w, h;
}
ADLWindowDef;

struct _ADLWindow
{
  ADLWindow *  parent;
  int          x, y;
  unsigned int w, h;
  bool         visible;

  bool haveMousePos;
  int  mouseX, mouseY;
  bool mouseWarping, mouseWarp;
};

ADL_STATUS adlWindowCreate(const ADLWindowDef def, ADLWindow ** result);
ADL_STATUS adlWindowDestroy(ADLWindow ** window);
ADL_STATUS adlWindowShow(ADLWindow * window);
ADL_STATUS adlWindowHide(ADLWindow * window);
ADL_STATUS adlWindowSetTitle(ADLWindow * window, const char * title);
ADL_STATUS adlWindowSetClassName(ADLWindow * window, const char * className);
ADL_STATUS adlWindowSetGrab(ADLWindow * window, bool enable);
ADL_STATUS adlWindowSetRelative(ADLWindow * window, bool enable);
ADL_STATUS adlWindowSetFocus(ADLWindow * window);

#endif
