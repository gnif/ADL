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

static int count = 0;

bool timerFunc(void * udata)
{
  printf("Tick %d\n", ++count);
  return true;
}

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

  ADLTimer timer;
  adlTimerCreate(1000, timerFunc, NULL, &timer);

  while(count < 10)
    usleep(100);

  printf("shutdown\n");
  adlTimerDestroy(&timer);
  adlShutdown();

err_exit:
  return retval;
}
