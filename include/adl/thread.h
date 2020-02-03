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

#ifndef _H_ADL_THREAD
#define _H_ADL_THREAD

#include "status.h"
#include <stdbool.h>
#include "platform.h"

typedef void * (*ADLThreadFn)(ADLThread * thread, void * udata);

/**
 * Create a new thread
 *
 * @param fn     The thread function
 * @param udata  Application defined data, may be NULL
 * @param result The new thread
 */
ADL_STATUS adlThreadCreate(ADLThreadFn fn, void * udata, ADLThread * result);

/**
 * Join a running or finished thread
 *
 * @param thread The thread to join
 * @param result The returned result from the thread
 * @param timeout The timeout in miliseconds to wait.
 *
 * If timeout zero the call is non-blocking and returns immediately, if timeout
 * is negative then the timeout is infinite.
 */
ADL_STATUS adlThreadJoin(ADLThread * thread, void ** result, int timeout);

/**
 * Signal the specified thread to gracefully exit.
 *
 * @param thread The thread to signal.
 */
void adlThreadStop(ADLThread * thread);

/**
 * Returns if the thread has been joined or stop has been called.
 *
 * @param thread The thread to check
 *
 * This function is intended to be used in a thread loop if the application has
 * one allowing the controlling thread to signal a graceful exit.
 */
bool adlThreadIsRunning(ADLThread * thread);

#endif
