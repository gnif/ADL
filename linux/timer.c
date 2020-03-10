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

#include "adl/timer.h"

#include <signal.h>
#include <time.h>

static void timerHandler(int sig, siginfo_t * si, void * uc)
{
  if (sig != SIGRTMIN || !si->si_value.sival_ptr)
    return;

  ADLTimer * timer = (ADLTimer *)si->si_value.sival_ptr;
  if (!((ADLTimerFn)timer->timerFn)(timer->udata))
    timer_delete(timer->timerID);
}

ADL_STATUS adlTimerCreate(const unsigned int intervalNS, ADLTimerFn fn,
    void * udata, ADLTimer * result)
{
  struct sigaction sa =
  {
    .sa_flags     = SA_SIGINFO,
    .sa_sigaction = timerHandler
  };

  result->timerFn = fn;
  result->udata   = udata;

  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGRTMIN, &sa, NULL) == -1)
    return ADL_ERR_PLATFORM;

  struct sigevent te =
  {
    .sigev_notify          = SIGEV_SIGNAL,
    .sigev_signo           = SIGRTMIN,
    .sigev_value.sival_ptr = result,
  };
  timer_create(CLOCK_REALTIME, &te, &result->timerID);

  struct itimerspec its =
  {
    .it_interval.tv_sec  = intervalNS / 1000000000U,
    .it_interval.tv_nsec = intervalNS - (intervalNS / 1000000000U * 1000000000U),
    .it_value.tv_sec     = intervalNS / 1000000000U,
    .it_value.tv_nsec    = intervalNS - (intervalNS / 1000000000U * 1000000000U)
  };
  timer_settime(result->timerID, 0, &its, NULL);

  return ADL_OK;
}

void adlTimerDestroy(ADLTimer * timer)
{
  timer_delete(timer->timerID);
}
