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

#ifndef _ADL_SRC_LOGGING
#define _ADL_SRC_LOGGING

#include <inttypes.h>

#include "adl/logging.h"

struct ADLLogHandlers adlLogHandlers;

#if defined(_WIN32) && !defined(__GNUC__)
  #define DIRECTORY_SEPARATOR '\\'
#else
  #define DIRECTORY_SEPARATOR '/'
#endif

#define STRIPPATH(s) ( \
  sizeof(s) >  2 && (s)[sizeof(s)- 3] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  2 : \
  sizeof(s) >  3 && (s)[sizeof(s)- 4] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  3 : \
  sizeof(s) >  4 && (s)[sizeof(s)- 5] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  4 : \
  sizeof(s) >  5 && (s)[sizeof(s)- 6] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  5 : \
  sizeof(s) >  6 && (s)[sizeof(s)- 7] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  6 : \
  sizeof(s) >  7 && (s)[sizeof(s)- 8] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  7 : \
  sizeof(s) >  8 && (s)[sizeof(s)- 9] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  8 : \
  sizeof(s) >  9 && (s)[sizeof(s)-10] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) -  9 : \
  sizeof(s) > 10 && (s)[sizeof(s)-11] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 10 : \
  sizeof(s) > 11 && (s)[sizeof(s)-12] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 11 : \
  sizeof(s) > 12 && (s)[sizeof(s)-13] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 12 : \
  sizeof(s) > 13 && (s)[sizeof(s)-14] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 13 : \
  sizeof(s) > 14 && (s)[sizeof(s)-15] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 14 : \
  sizeof(s) > 15 && (s)[sizeof(s)-16] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 15 : \
  sizeof(s) > 16 && (s)[sizeof(s)-17] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 16 : \
  sizeof(s) > 17 && (s)[sizeof(s)-18] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 17 : \
  sizeof(s) > 18 && (s)[sizeof(s)-19] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 18 : \
  sizeof(s) > 19 && (s)[sizeof(s)-20] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 19 : \
  sizeof(s) > 20 && (s)[sizeof(s)-21] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 20 : \
  sizeof(s) > 21 && (s)[sizeof(s)-22] == DIRECTORY_SEPARATOR ? (s) + sizeof(s) - 21 : (s))

#define DEBUG_PRINT(type, level, status, fmt, ...) \
  do { \
    adlLogHandlers.type(\
      level, \
      status, \
      STRIPPATH(__FILE__), \
      __LINE__, \
      __FUNCTION__, \
      fmt "\n", \
      ##__VA_ARGS__); \
  } while (0)

#define DEBUG_INFO(status, fmt, ...)  \
  DEBUG_PRINT(info , ADL_LOG_INFO , status, fmt, ##__VA_ARGS__)
#define DEBUG_WARN(status, fmt, ...)  \
  DEBUG_PRINT(warn , ADL_LOG_WARN , status, fmt, ##__VA_ARGS__)
#define DEBUG_ERROR(status, fmt, ...) \
  DEBUG_PRINT(err  , ADL_LOG_ERROR, status, fmt, ##__VA_ARGS__)
#define DEBUG_BUG(status, fmt, ...)   \
  DEBUG_PRINT(bug  , ADL_LOG_BUG  , status, fmt, ##__VA_ARGS__)
#define DEBUG_FATAL(status, fmt, ...) \
  DEBUG_PRINT(fatal, ADL_LOG_FATAL, status, fmt, ##__VA_ARGS__)

#endif
