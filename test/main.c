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
  ADLWindow window;
  ADLWindowDef def =
  {
    .x = 0,
    .y = 0,
    .w = 100,
    .h = 100
  };
  if (adlWindowCreate(def, &window) != ADL_OK)
  {
    retval = -1;
    goto err_shutdown;
  }

  adlWindowShow(window);

  /* Process events */
  ADLEvent event;
  ADL_STATUS status;
  while((status = adlProcessEvents(&event)) == ADL_OK)
  {
    if (event.type == ADL_EVENT_NONE)
      continue;

    if (event.type == ADL_EVENT_CLOSE)
      if (event.u.close.window == window)
        break;
  }

  /* cleanup */
  adlWindowDestroy(&window);
err_shutdown:
  adlShutdown();
err_exit:
  return retval;
}
