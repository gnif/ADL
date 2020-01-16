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

int main()
{
  int retval = 0;

  if (adlInitialize() != ADL_OK)
  {
    retval = -1;
    goto err_exit;
  }

  /* At this point we are certain there is atleast one platform as adlInitialize
   * wont return ADL_OK otherwise. ADL will also fail to compile if all
   * platforms are disabled */
  {
    int count;
    adlGetPlatformList(&count, NULL);

    const char * platforms[count];
    adlGetPlatformList(&count, platforms);

    /* Use the first available platform, in reality you would likely loop through
     * the list to try each platform until one succeeds, this has been left to
     * the calling application so that a specific platform can be selected if
     * required */
    if (adlUsePlatform(platforms[0]) != ADL_OK)
    {
      retval = -1;
      goto err_exit;
    }
  }

  /* Create a window and show it */
  ADLWindowDef def =
  {
    .title       = "ADL Test",
    .className   = "adl-test",
    .type        = ADL_WINDOW_TYPE_NORMAL,
    .flags       = 0,
    .borderless  = false,
    .x           = 0  , .y = 0  ,
    .w           = 200, .h = 200,
  };
  ADLWindow * window;
  if (adlWindowCreate(def, &window) != ADL_OK)
  {
    retval = -1;
    goto err_shutdown;
  }

  adlWindowShow(window);
  adlFlush();

  /* Process events */
  ADLEvent event;
  ADL_STATUS status;
  while((status = adlProcessEvent(100, &event)) == ADL_OK)
  {
    switch(event.type)
    {
      case ADL_EVENT_NONE:
        continue;

      case ADL_EVENT_CLOSE:
        printf("close event\n");
        goto exit;

      case ADL_EVENT_SHOW:
        printf("show event\n");
        break;

      case ADL_EVENT_HIDE:
        printf("hide event\n");
        break;

      case ADL_EVENT_WINDOW_CHANGE:
        printf("change event: x:%-4d y:%-4d w:%-4d h:%-4d\n",
            event.u.win.x, event.u.win.y,
            event.u.win.w, event.u.win.h);
        break;

      case ADL_EVENT_KEY_DOWN:
        printf("key down: %u\n", event.u.key.scancode);
        break;

      case ADL_EVENT_KEY_UP:
        printf("key up: %u\n", event.u.key.scancode);
        if (event.u.key.scancode == 9)
          goto exit;
        break;

      case ADL_EVENT_MOUSE_DOWN:
        printf("down: %4d %4d %08x\n", event.u.mouse.x, event.u.mouse.y,
            event.u.mouse.buttons);
        break;

      case ADL_EVENT_MOUSE_UP:
        printf("up  : %4d %4d %08x\n", event.u.mouse.x, event.u.mouse.y,
            event.u.mouse.buttons);
        break;

      case ADL_EVENT_MOUSE_MOVE:
        printf("move: %4d %4d %4d %4d %08x\n", event.u.mouse.x, event.u.mouse.y,
            event.u.mouse.relX, event.u.mouse.relY,
            event.u.mouse.buttons);
        break;

      default:
        printf("Unhandled message: %u\n", event.type);
        break;
    }
  }

exit:
  printf("shutdown\n");
err_shutdown:
  adlShutdown();
err_exit:
  return retval;
}
