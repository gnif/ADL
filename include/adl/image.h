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

#ifndef _H_ADL_IMAGE
#define _H_ADL_IMAGE

#include "window.h"

#include <sys/types.h>

typedef enum
{
  ADL_IMAGE_BACKEND_DMABUF = 0x1,
  ADL_IMAGE_BACKEND_BUFFER = 0x2
}
ADLImageBackend;

typedef enum
{
  ADL_IMAGE_FORMAT_RGBA,
  ADL_IMAGE_FORMAT_BGRA
}
ADLImageFormat;

typedef struct
{
  int    fd    ; // the dma file descriptor
}
ADLImageDMABuf;

typedef void * ADLImageBuffer;

typedef struct
{
  ADLImageBackend backend; // the image storage backend
  ADLImageFormat  format;  // the image format
  unsigned int    bpp;     // ie: 24 for 24 in 32-bit
  unsigned int    depth;   // ie: 32 for 24 in 32-bit
  unsigned int    pitch;   // number of bytes in a single row including padding
  unsigned int    w;       // width
  unsigned int    h;       // height
  union
  {
    ADLImageDMABuf dmabuf;
    ADLImageBuffer buffer;
  }
  u;
}
ADLImageDef;

/* everything in the structure is read-only to the application! */
typedef struct
{
  ADLWindow *  window;
  unsigned int w;
  unsigned int h;
}
ADLImage;

/* get the backends supported by the platform */
ADL_STATUS adlImageGetSupported(const ADLImageBackend ** backends);

/* create a new image per the definition */
ADL_STATUS adlImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage ** image);

/* destroy a image */
ADL_STATUS adlImageDestroy(ADLImage ** image);

/* update the image from it's backend storage */
ADL_STATUS adlImageUpdate(ADLImage * image);

#endif
