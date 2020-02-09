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

#include "adl.h"
#include "adl/util.h"

#include <assert.h>
#include <time.h>

uint64_t adlGetClockMS(void)
{
  struct timespec time;
  assert(clock_gettime(CLOCK_MONOTONIC, &time) == 0);
  return ((uint64_t)time.tv_sec * 1000LLU + time.tv_nsec / 1000000LLU) -
    adl.startTime;
}

uint64_t adlGetClockNS(void)
{
  struct timespec time;
  assert(clock_gettime(CLOCK_MONOTONIC, &time) == 0);
  return ((uint64_t)time.tv_sec * 1000000000LLU + time.tv_nsec) -
    adl.startTime * 1000000LLU;
}

void adlWaitUntilMS(uint64_t clockMS)
{
  clockMS += adl.startTime;
  const struct timespec time =
  {
    .tv_sec  = clockMS / 1000LLU,
    .tv_nsec = (clockMS % 1000LLU) * 1000LLU
  };
  while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL) != 0) {}
}

void adlWaitUntilNS(uint64_t clockNS)
{
  clockNS += adl.startTime * 1000000LLU;
  const struct timespec time =
  {
    .tv_sec  = clockNS / 1000000000LLU,
    .tv_nsec = clockNS % 1000000000LLU
  };
  while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL) != 0) {}
}
