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

#ifndef _FLOTT_OUTPUT_H_
#define _FLOTT_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/**
 * constant 'define' macros
 */
#define FLOTT_COLFORM_BUFSZ 64      ///< size of number format buffer

/**
 * user type definitions for console application
 */
typedef enum flott_output_options flott_output_options;
typedef struct flott_user_output flott_user_output;

enum flott_output_options
{
  FLOTT_OUT_INPUT_OFFSET             = 1,
  FLOTT_OUT_T_AUG_LEVEL              = 1 << 2,
  FLOTT_OUT_CF                       = 1 << 3,
  FLOTT_OUT_CP_OFFSET                = 1 << 4,
  FLOTT_OUT_CP_LENGTH                = 1 << 5,
  FLOTT_OUT_T_COMPLEXITY             = 1 << 6,
  FLOTT_OUT_T_INFORMATION            = 1 << 7,
  FLOTT_OUT_AVE_T_ENTROPY            = 1 << 8,
  FLOTT_OUT_INST_T_ENTROPY           = 1 << 9,
  FLOTT_OUT_CP_STRING                = 1 << 10,
  FLOTT_OUT_NTI_DIST                 = 1 << 11,
  FLOTT_OUT_NTC_DIST                 = 1 << 12,

  FLOTT_OUT_STEP                     = ( FLOTT_OUT_INPUT_OFFSET
                                        | FLOTT_OUT_CF
                                        | FLOTT_OUT_CP_OFFSET
                                        | FLOTT_OUT_CP_LENGTH
                                        | FLOTT_OUT_INST_T_ENTROPY
                                        | FLOTT_OUT_CP_STRING ),

  FLOTT_OUT_CONCAT_INPUT             = 1 << 13,
  FLOTT_OUT_UNITS_BITS               = 1 << 14,
  FLOTT_OUT_HEADERS                  = 1 << 15,

  FLOTT_OUT_PRETTY                   = 1 << 16,
  FLOTT_OUT_CSV                      = 1 << 17,
  FLOTT_OUT_TAB                      = 1 << 18
};

struct flott_user_output
{
  flott_output_options options; ///< output options bit field
  int progress_bar_length;

  int precision;         ///< number of decimal digits
  double scale_factor;   ///< t-information, t-entropy bits/nats scale factor
  double previous_t_information;
  size_t previous_input_offset;

  char column_separator; ///< column separator character
  char column_header[FLOTT_LINE_BUFSZ];   ///< buffer for column header string
  char line_buffer[FLOTT_LINE_BUFSZ];   ///< buffer for column header string
  char basic_int[FLOTT_COLFORM_BUFSZ];    ///< buffer for 'int' format string
  char basic_double[FLOTT_COLFORM_BUFSZ]; ///< buffer for 'double' format string
  char short_double[FLOTT_COLFORM_BUFSZ]; ///< format buffer of t-entropy format string

  flott_storage_type storage_type; ///< device type the output is written to
  char* path;
  FILE *handle; ///< output device file handle
};

/**
 * function prototypes
 */
int flott_set_output_device (flott_object *op);
int flott_output_initialize (flott_object *op);
void flott_output_print_headers (const flott_object *op);
int flott_output_nti_dist (flott_object *op);
int flott_output_ntc_dist (flott_object *op);
void flott_output_no_rate (flott_object *op);
void flott_output_step (flott_object *op, flott_token* cp_last, const flott_uint level,
                        const size_t cf_value, const size_t cp_start_offset,
                        const size_t cp_length, const size_t joined_cp_length,
                        const double t_complexity, int *terminate);
int flott_output (flott_object *op);
void flott_output_destroy (const flott_object *op);
void flott_output_progress_bar (const flott_object *op, const float ratio);

#endif /* _FLOTT_OUTPUT_H_ */
