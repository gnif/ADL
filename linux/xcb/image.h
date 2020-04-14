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

#ifndef _H_LINUX_XCB_IMAGE
#define _H_LINUX_XCB_IMAGE

#include "adl/status.h"
#include "adl/image.h"

#include <xcb/render.h>

typedef struct
{
  ADLWindow *  window;
  ADLImageDef  def;

  xcb_render_pictforminfo_t * format;

  xcb_pixmap_t pixmap;
  unsigned int serial;
}
ImageData;

ADL_STATUS xcbImageGetSupported(const ADLImageBackend ** result);
ADL_STATUS xcbImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage * result);
ADL_STATUS xcbImageDestroy(ADLImage * image);
ADL_STATUS xcbImageUpdate(ADLImage * image);

#endif
