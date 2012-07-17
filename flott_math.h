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

#ifndef _FLOTT_MATH_H_
#define _FLOTT_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "flott.h"
#include "flott_config.h"

#ifdef FLOTT_USE_LOG2_LUT
#define flott_log2_M(x) flott_log2(x)
#include "flott_math.def"

#ifndef _MSC_VER
  static FLOTT_INLINE double flott_log2 (const size_t x) __attribute__((always_inline));
#endif

static FLOTT_INLINE double
flott_log2 (const size_t x)
{
  static const int64_t log2_lut[4096] = FLOTT_LOG2_LUT_4096;
  if (x < 4096)
    {
      /* some compilers may complain here, as we are casting an 64-bit integer
       * pointer to a double pointer, however this is legitimate as long as
       * the underlying architecture uses 64-bit doubles in accordance with
       * the IEEE 754 floating point specification */
      return ((double*) log2_lut)[x];
    }
  else
    {
      /* printf("log2 lut miss...\n"); */
      #ifdef _MSC_VER
	    return (log ((double)(x)) / log (2.0));
	  #else
	    return log2 ((double) x);
	  #endif /* _MSC_VER */
    }
}
#else
/* use native log routine and generate compiler warning */
#define FLOTT_LOG2_WARNING \
	"not using log2 lookup table, this will degrade performance."
#ifdef _MSC_VER
  #pragma message (FLOTT_LOG2_WARNING)
  #define flott_log2_M(x) (log ((double)(x)) / log (2.0))
#else
  #warning FLOTT_LOG2_WARNING
  #define flott_log2_M(x) log2 ((double)(x))
#endif /* _MSC_VER */
#endif /* FLOTT_USE_LOG2_LUT */

#ifdef __cplusplus
}
#endif

double flott_get_t_information (double t_complexity);

#endif /* _FLOTT_MATH_H_ */

