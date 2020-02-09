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

#include "adl/logging.h"
#include "adl.h"

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#define ADL_LOG_FORMAT "%16" PRId64 " | %20s:%-4u | %-30s | "

static void adlLogStdout(
  ADL_LOG_LEVEL loglevel,
  ADL_STATUS    status,
  const char *  file,
  unsigned int  line,
  const char *  function,
  const char *  format,
  ...)
{
  fprintf(stdout, ADL_LOG_FORMAT, adlGetClockMS(), file, line, function);

  va_list ap;
  va_start(ap, format);
  vfprintf(stdout, format, ap);
  va_end(ap);
}

static void adlLogStderr(
  ADL_LOG_LEVEL loglevel,
  ADL_STATUS    status,
  const char *  file,
  unsigned int  line,
  const char *  function,
  const char *  format,
  ...)
{
  fprintf(stderr, ADL_LOG_FORMAT, adlGetClockMS(), file, line, function);

  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

struct ADLLogHandlers adlLogHandlers =
{
  .info  = adlLogStdout,
  .warn  = adlLogStderr,
  .bug   = adlLogStderr,
  .err   = adlLogStderr,
  .fatal = adlLogStderr
};

void ADLSetLogHandlers(const struct ADLLogHandlers log)
{
  adlLogHandlers = log;
}
