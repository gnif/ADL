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

#ifndef _H_INTERFACE_ADL
#define _H_INTERFACE_ADL

#include "adl/adl.h"
#include "adl/status.h"

#include <stdbool.h>
#include <stddef.h>

#if defined(ADL_HAS_EGL)
#include <EGL/egl.h>
#endif

/* platform functions */
typedef ADL_STATUS (*ADLPf)(void);
typedef ADL_STATUS (*ADLPfProcessEvent)
  (int timeout, ADLEvent * event);

/* window functions */
typedef ADL_STATUS (*ADLPfWindowCreate)(const ADLWindowDef def,
    ADLWindow * result);
typedef ADL_STATUS (*ADLPfWindow)(ADLWindow * window);
typedef ADL_STATUS (*ADLPfWindowEvent   )(ADLWindow * window, ADLEvent * event);
typedef ADL_STATUS (*ADLPfWindowSetStr  )(ADLWindow * window, const char * str);
typedef ADL_STATUS (*ADLPfWindowSetBool )(ADLWindow * window, bool enable     );

/* image functions */
typedef ADL_STATUS (*ADLPfImageGetSupported)(const ADLImageBackend ** result);
typedef ADL_STATUS (*ADLPfImageCreate      )(ADLWindow * window,
    const ADLImageDef def, ADLImage * result);
typedef ADL_STATUS (*ADLPfImage            )(ADLImage * result);

/* pointer functions */
typedef ADL_STATUS (*ADLPfPointer)(ADLWindow * window, int x, int y);
typedef ADL_STATUS (*ADLPfPointerCursor)(ADLWindow * window, ADLImage * source,
    ADLImage * mask, int x, int y);

#if defined(ADL_HAS_EGL)
/* egl functions */
typedef ADL_STATUS (*ADLPfEGLGetDisplay)(EGLDisplay ** display);
typedef ADL_STATUS (*ADLPfEGLCreateWindowSurface)(EGLDisplay * display,
  EGLint * config, ADLWindow * window, const EGLint * attribs,
  EGLSurface * surface);
#endif

#define ADL_PLATFORM_FIELDS \
  ADL_FIELD(const char *     , name        ) \
  ADL_FIELD(ADLPf            , test        ) \
  ADL_FIELD(ADLPf            , init        ) \
  ADL_FIELD(ADLPf            , deinit      ) \
  ADL_FIELD(ADLPfProcessEvent, processEvent) \
  ADL_FIELD(ADLPf            , flush       ) \
  \
  ADL_FIELD(size_t            , windowDataSize    ) \
  ADL_FIELD(ADLPfWindowCreate , windowCreate      ) \
  ADL_FIELD(ADLPfWindow       , windowDestroy     ) \
  ADL_FIELD(ADLPfWindow       , windowShow        ) \
  ADL_FIELD(ADLPfWindow       , windowHide        ) \
  ADL_FIELD(ADLPfWindowSetStr , windowSetTitle    ) \
  ADL_FIELD(ADLPfWindowSetStr , windowSetClassName) \
  ADL_FIELD(ADLPfWindowSetBool, windowSetGrab     ) \
  ADL_FIELD(ADLPfWindowSetBool, windowSetRelative ) \
  ADL_FIELD(ADLPfWindow       , windowSetFocus    ) \
  ADL_FIELD(ADLPfWindowEvent  , windowEvent       ) \
  \
  ADL_FIELD(size_t                , imageDataSize    ) \
  ADL_FIELD(ADLPfImageGetSupported, imageGetSupported) \
  ADL_FIELD(ADLPfImageCreate      , imageCreate      ) \
  ADL_FIELD(ADLPfImage            , imageDestroy     ) \
  ADL_FIELD(ADLPfImage            , imageUpdate      ) \
  \
  ADL_FIELD(ADLPfPointer      , pointerWarp     ) \
  ADL_FIELD(ADLPfWindowSetBool, pointerVisible  ) \
  ADL_FIELD(ADLPfPointerCursor, pointerSetCursor) \
  \
  ADL_EGL_FIELD(ADLPfEGLGetDisplay         , eglGetDisplay         ) \
  ADL_EGL_FIELD(ADLPfEGLCreateWindowSurface, eglCreateWindowSurface)

#define ADL_FIELD(type, name) type name;

#if defined(ADL_HAS_EGL)
  #define ADL_EGL_FIELD(type, name) ADL_FIELD(type, name)
#else
  #define ADL_EGL_FIELD(type, name)
#endif

struct ADLPlatform
{
  ADL_PLATFORM_FIELDS
};

#undef ADL_FIELD

#define adl_platform(x) \
  static const void * __adl_platform_p_##x = &x;\
  static const void ** __adl_platform_##x \
  __attribute__((__used__)) \
  __attribute__((__section__("adl_platforms"))) = &__adl_platform_p_##x;

#endif
