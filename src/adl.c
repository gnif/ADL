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
#include "logging.h"

#include "interface/adl.h"

#include "adl/adl.h"
#include "adl/status.h"

#include <stdint.h>
#include <unistd.h>
#include <string.h>

struct ADL adl = { 0 };

ADL_STATUS adlInitialize()
{
  ADL_STATUS status = ADL_OK;

  /* get the platform list and count */
  extern const void * __start_adl_platforms;
  extern const void * __stop_adl_platforms;

  adl.platformListCount = ((uintptr_t)&__stop_adl_platforms -
    (uintptr_t)&__start_adl_platforms) / sizeof(uintptr_t);
  adl.platformList      =
    (const struct ADLPlatform **)__start_adl_platforms;

  if (adl.initDone)
  {
    DEBUG_ERROR(ADL_ERR_ALREADY_INITIALIZED, "already initialized");
    return ADL_ERR_ALREADY_INITIALIZED;
  }

  /* call the platforms test methods, removing any that fail or are
   * unsupported to prevent future evaluation */
  for(int i = 0; i < adl.platformListCount; ++i)
  {
    const struct ADLPlatform * p = adl.platformList[i];

    /* perform a sanity check on the struct */
    if (
      !p->test          ||
      !p->init          ||
      !p->deinit        ||
      !p->processEvents ||
      !p->windowCreate  ||
      !p->windowDestroy ||
      !p->windowShow    ||
      !p->windowHide    ||
      0
    ) {
      DEBUG_BUG(ADL_ERR_PLATFORM,
          "Platform `%s` is incomplete", p->name);
      adl.platformList[i] = NULL;
      continue;
    }

    if ((status = p->test()) == ADL_OK)
    {
      ++adl.numPlatforms;
      continue;
    }

    /* not an error, the platform is just not supported */
    if (status == ADL_ERR_UNSUPPORTED)
    {
      adl.platformList[i] = NULL;
      continue;
    }

    /* treat everything else as a warning */
    DEBUG_WARN(status,
        "Platform `%s` failed with the error: %s",
        p->name,
        adlStatusString(status));
    adl.platformList[i] = NULL;
  }

  DEBUG_INFO(status,
      "%d platform(s) available", adl.numPlatforms);

  if (adl.numPlatforms == 0)
  {
    status = ADL_ERR_UNSUPPORTED;
    DEBUG_ERROR(status,
      "Unable to start, no supported platforms found");
  }

  adl.initDone = true;
  return status;
}

ADL_STATUS adlShutdown()
{
  ADL_INITCHECK;

  return ADL_OK;
}

ADL_STATUS adlGetPlatformList(int * count, const char * names[])
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(count);

  *count = adl.numPlatforms;
  if (!names)
    return ADL_OK;

  for(int i = 0; i < adl.platformListCount; ++i)
  {
    if (!adl.platformList[i])
      continue;
    *names = adl.platformList[i]->name;
    ++names;
  }

  return ADL_OK;
}


ADL_STATUS adlUsePlatform(const char * name)
{
  ADL_INITCHECK;

  for(int i = 0; i < adl.platformListCount; ++i)
  {
    if (!adl.platformList[i])
      continue;

    if (strcmp(adl.platformList[i]->name, name) == 0)
    {
      adl.platform = adl.platformList[i];
      break;
    }
  }

  if (!adl.platform)
  {
    DEBUG_ERROR(ADL_ERR_INVALID_PLATFORM, "The platform `%s` is unknown", name);
    return ADL_ERR_INVALID_PLATFORM;
  }

  ADL_STATUS status;
  if ((status = adl.platform->init()) != ADL_OK)
  {
    DEBUG_ERROR(status, "Platform `%s` initialization failed", name);
    return status;
  }

  DEBUG_INFO(ADL_OK, "Using platform: %s", name);
  return ADL_OK;
}

ADL_STATUS adlWindowCreate(const ADLWindowDef def, ADLWindow * result)
{
  ADL_INITCHECK;
  ADL_STATUS status;

  if (!result)
  {
    DEBUG_ERROR(ADL_ERR_INVALID_ARGUMENT, "result == NULL");
    return ADL_ERR_INVALID_ARGUMENT;
  }

  *result = NULL;
  status  = adl.platform->windowCreate(def, result);
  if (status == ADL_OK)
  {
    if (!*result)
      DEBUG_BUG(ADL_ERR_PLATFORM,
          "%s->windowCreate did not return a result", adl.platform->name);
  }
  else
  {
    *result = NULL;
  }

  return status;
}

ADL_STATUS adlProcessEvents(ADLEvent * event)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(event);
  return adl.platform->processEvents(event);
}

ADL_STATUS adlWindowDestroy(ADLWindow * window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);

  if (!*window)
    return ADL_OK;

  ADL_STATUS status;
  status = adl.platform->windowDestroy(*window);
  *window = NULL;

  return status;
}

ADL_STATUS adlWindowShow(ADLWindow window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowShow(window);
}

ADL_STATUS adlWindowHide(ADLWindow window)
{
  ADL_INITCHECK;
  ADL_NOT_NULL_CHECK(window);
  return adl.platform->windowHide(window);
}
