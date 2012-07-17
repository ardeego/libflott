/*
 * Copyright 2012 Niko Rebenich and Stephen Neville,
 *                University of Victoria
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _FLOTT_PLATFORM_H_
#define _FLOTT_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
  /* this section of 'defines' might apply to other non-C99 compilers as well */
  #include <windows.h>

  typedef __int32 int32_t;
  typedef unsigned __int32 uint32_t;
  typedef __int64 int64_t;
  typedef unsigned __int64 uint64_t;

  typedef enum {flott_false, flott_true} flott_bool;
  #define bool flott_bool
  #define true flott_true
  #define false flott_false

  #ifndef UINT32_MAX
    #define UINT32_MAX  ((uint32_t)-1)
  #endif

  #define ANSI
  #include <stdarg.h>
  #ifndef va_copy
    #define va_copy(a, b) ((a) = (b))
  #endif

  #define FLOTT_PRINTF_T_SIZE_T     "Iu"
#else
  #include <stddef.h>
  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>

  #define FLOTT_PRINTF_T_SIZE_T     "zu"
#endif /* _MSC_VER */


#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_PATFORM_H_ */
