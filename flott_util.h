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

#ifndef _FLOTT_UTIL_H_
#define _FLOTT_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "flott_platform.h"

int flott_file_exists (char *filename);
int flott_get_file_size (char *filename);
int flott_file_exists_not_empty (char *filename);
int flott_load_file_to_memory(const char *filename, char **result);
int flott_get_digit_count (uint64_t uint_val);

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_UTIL_H_ */
