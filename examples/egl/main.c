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

#include <adl/adl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <EGL/egl.h>
#include "gl.h"

#define TARGET_FPS 60

int winW = 400;
int winH = 400;

EGLDisplay * display;
EGLContext   context;
EGLSurface   surface;

const char * vertShaderSrc =
  "attribute vec4 vPosition;\n"
  "void main()\n"
  "{\n"
  "  gl_Position = vPosition;\n"
  "}";

const char * fragShaderSrc =
  "precision mediump float;\n"
  "void main()\n"
  "{\n"
  "  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
  "}";

GLuint loadShader(const char * src, GLenum type)
{
  GLuint shader;
  GLint  compiled;

  shader = glCreateShader(type);
  if (shader == 0)
    return 0;

  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

void * renderThread(ADLThread * thread, void * opaque)
{
  if (!eglMakeCurrent(display, surface, surface, context))
  {
    printf("eglMakeCurrent failed\n");
    return NULL;
  }

  GLuint vertShader = loadShader(vertShaderSrc, GL_VERTEX_SHADER  );
  if (!vertShader)
  {
    printf("Failed to load the vertex shader\n");
    return NULL;
  }

  GLuint fragShader = loadShader(fragShaderSrc, GL_FRAGMENT_SHADER);
  if (!fragShader)
  {
    printf("Failed to load the fragment shader\n");
    return NULL;
  }

  GLuint program    = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glBindAttribLocation(program, 0, "vPosition");
  glLinkProgram(program);

  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    glDeleteProgram(program);
    printf("Failed to link\n");
    return NULL;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  const GLfloat vVertices[] =
  {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
  };

  while(adlThreadIsRunning(thread))
  {
    const int64_t start = adlGetClockNS();

    glViewport(0, 0, winW, winH);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(display, surface);

    adlWaitUntilNS(start + (1000000000LL / TARGET_FPS));
  }

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  glDeleteProgram(program);
  return NULL;
}

int main(int argc, char * argv[])
{
  int retval = 0;
  if (adlInitialize() != ADL_OK)
  {
    retval = -1;
    goto err_exit;
  }

  {
    int count;
    adlGetPlatformList(&count, NULL);

    const char * platforms[count];
    adlGetPlatformList(&count, platforms);
    if (adlUsePlatform(platforms[0]) != ADL_OK)
    {
      printf("adlUsePlatform failed\n");
      retval = -1;
      goto err_exit;
    }
  }

  ADLWindowDef winDef =
  {
    .title       = "ADL EGL Test",
    .className   = "adl-egl-test",
    .type        = ADL_WINDOW_TYPE_NORMAL,
    .flags       = 0,
    .borderless  = false,
    .x           = 0   , .y = 0   ,
    .w           = winW, .h = winH
  };

  ADLWindow * window;
  if (adlWindowCreate(winDef, &window) != ADL_OK)
  {
    printf("adlWindowCreate failed\n");
    retval = -1;
    goto err_shutdown;
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API))
  {
    printf("eglBindAPI failed\n");
    retval = -1;
    goto exit;
  }

  if (adlEGLGetDisplay(&display) != ADL_OK)
  {
    printf("adlEGLGetDisplay failed\n");
    retval = -1;
    goto exit;
  }

  EGLint minor, major;
  if (!eglInitialize(display, &minor, &major))
  {
    printf("eglInitialize failed\n");
    retval = -1;
    goto exit;
  }

  printf("EGL: %d.%d\n", minor, major);

  const EGLint configAttribs[] =
  {
    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
    EGL_BUFFER_SIZE      , 32,
    EGL_RED_SIZE         , 8,
    EGL_GREEN_SIZE       , 8,
    EGL_BLUE_SIZE        , 8,
    EGL_ALPHA_SIZE       , 8,
    EGL_DEPTH_SIZE       , 24,
    EGL_STENCIL_SIZE     , 8,
    EGL_SAMPLE_BUFFERS   , 0,
    EGL_SAMPLES          , 0,
    EGL_SURFACE_TYPE     , EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE  , EGL_OPENGL_ES2_BIT,
    EGL_NONE
  };

  EGLConfig * configs = malloc(sizeof(EGLConfig) * 256);
  EGLint      numConfigs;

  if (!eglChooseConfig(
    display,
    configAttribs,
    configs,
    256,
    &numConfigs))
  {
    printf("eglChooseConfig failed\n");
    retval = -1;
    goto exit;
  }

  if (numConfigs == 0)
  {
    free(configs);
    printf("eglChooseConfig, config unsupported\n");
    retval = -1;
    goto exit;
  }

  const EGLint contextAttribs[] =
  {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  context = eglCreateContext(
    display, configs[0], EGL_NO_CONTEXT, contextAttribs);

  if (!context)
  {
    free(configs);
    printf("eglCreateContext failed\n");
    retval = -1;
    goto exit;
  }

  const EGLint surfaceAttribs[] =
  {
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_NONE
  };

  if (adlEGLCreateWindowSurface(
    display, configs[0], window, surfaceAttribs, &surface) != ADL_OK)
  {
    free(configs);
    printf("adlEGLCreateWindowSurface failed\n");
    retval = -1;
    goto exit;
  }

  free(configs);

  ADLThread thread;
  adlThreadCreate(renderThread, NULL, &thread);

  /* show the window */
  adlWindowShow(window);
  adlFlush();

  /* Process events */
  ADLEvent event;
  ADL_STATUS status;
  while((status = adlProcessEvent(1, &event)) == ADL_OK)
  {
    switch(event.type)
    {
      case ADL_EVENT_CLOSE:
        printf("close event\n");
        goto exit;

      case ADL_EVENT_WINDOW_CHANGE:
        winW = event.u.win.w;
        winH = event.u.win.h;
        break;

      default:
        break;
    }
  }

exit:
  printf("shutdown\n");
  adlThreadStop(&thread);
  adlThreadJoin(&thread, NULL, -1);
err_shutdown:
  adlShutdown();
err_exit:
  return retval;
}
