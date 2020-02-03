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

#include "adl/status.h"

const char * adlStatusString(const ADL_STATUS status)
{
  switch(status)
  {
    case ADL_OK                     : return "ADL_OK";
    case ADL_ERR_UNSUPPORTED        : return "ADL_ERR_UNSUPPORTED";
    case ADL_ERR_NOT_INITIALIZED    : return "ADL_ERR_NOT_INITIALIZED";
    case ADL_ERR_ALREADY_INITIALIZED: return "ADL_ERR_ALREADY_INITIALIZED";
    case ADL_ERR_INVALID_ARGUMENT   : return "ADL_ERR_INVALID_ARGUMENT";
    case ADL_ERR_INVALID_PLATFORM   : return "ADL_ERR_INVALID_PLATFORM";
    case ADL_ERR_PLATFORM           : return "ADL_ERR_PLATFORM";
    case ADL_ERR_NO_MEM             : return "ADL_ERR_NO_MEM";
    case ADL_ERR_EMPTY              : return "ADL_ERR_EMPTY";
    case ADL_ERR_FULL               : return "ADL_ERR_FULL";
    case ADL_ERR_UNSUPPORTED_BACKEND: return "ADL_ERR_UNSUPPORTED_BACKEND";
    case ADL_ERR_UNSUPPORTED_FORMAT : return "ADL_ERR_UNSUPPORTED_FORMAT";
    case ADL_ERR_BUSY               : return "ADL_ERR_BUSY";
    case ADL_ERR_TIMEOUT            : return "ADL_ERR_TIMEOUT";
  }

  return "BUG: Invalid or Unknown ADL_STATUS";
}
