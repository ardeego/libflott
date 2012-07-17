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

#ifndef _FLOTT_ERROR_H_
#define _FLOTT_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FLOTT_MAX_ERROR_CODES   21

typedef enum flott_error_codes flott_error_codes;

/**
 * flott error codes
 */
enum flott_error_codes
{
  FLOTT_SUCCESS               =   0,
  FLOTT_ERROR                 =  -1,
  FLOTT_CUSTOM_ERROR          =  -2,


  FLOTT_ERR_MALLOC_FLOTT      = -11,
  FLOTT_ERR_CREATE_OBJ        = -12,
  FLOTT_ERR_MAX_INPUT_SIZE    = -13,
  FLOTT_ERR_INVALID_OPT       = -14,
  FLOTT_ERR_INVALID_OBJ       = -15,
  FLOTT_ERR_INDEX_BOUNDS      = -16,
  FLOTT_ERR_LOADING_FILE      = -17,
  FLOTT_ERR_FILE_NOT_FOUND    = -18,
  FLOTT_ERR_NULL_POINTER      = -19,
  FLOTT_ERR_NID_NUM_INPUTS    = -20
};

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_ERROR_H_ */
