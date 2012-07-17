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

#include "flott_lang.h"
#include "flott_error.h"

/**
 * flott help menu
 */
const char* flott_msg_help_G =
  "FAST LOW MEMORY T-TRANSFORM (FLOTT)\n"
  "===================================\n"
  "\n"
  "usage: flott [option] [argument]\n"
  "\nSWITCHES:\n"
  "   -h              help (this screen)\n"
  "   -d              output normalized T-information distance\n"
  "   -D              output normalized T-complexity distance\n"
  "   -c              output T-complexity\n"
  "   -i              output T-information\n"
  "   -e              output average T-entropy rate\n"
  "   -r              output instantaneous T-entropy rate\n"
  "   -x              output interpolated rates over entire input\n"
  "   -n              output T-transform level\n"
  "   -k              output copy factor (a.k.a. T-expansion parameter)\n"
  "   -p              output copy pattern string\n"
  "                   (8-bit wide symbols only -- slow, don't use for long strings)\n"
  "   -o              output copy pattern start offset in string\n"
  "   -l              output copy pattern length\n"
  "   -u=[units]      output units: [bits, nats]; (default: bits)\n"
  "\nINPUT:\n"
  "   -I filename     set input filename (multiple allowed)\n"
  "   -S \"string\"     set input string (multiple allowed, enclose in quotes)\n"
  "   -b[bits]        set input symbol width in bits: [1, 8]; (default: 8)\n"
  "   -j              concatenate input files/strings (order: left-to-right)\n"
  "   -z              append terminal (dummy) character to input\n"
  "   -m              buffer input in memory\n"
  "\nOUTPUT:\n"
  "   -O filename     set output filename\n"
  "   -F=[format]     output format options: [pretty, csv, tab]; (default: pretty)\n"
  "   -L              output column labels\n"
  "   -g              floating point precision: [0 - 100] (default: 2)\n"
  "   -q              quiet, omit status information (equivalent to -v0)\n"
  "   -v[level]       verbosity level: [0 - 5]; (default: 1, quiet: 0)\n";

/**
 * flott error message look up table
 */
const char* flott_errmsg_lut_G[FLOTT_MAX_ERROR_CODES] =
{
  "", /* FLOTT_SUCCESS */
  "unknown error occurred.",
  "", /* FLOTT_CUSTOM_ERROR */
  "", /* reserved error no. */
  "", /*         ||         */
  "", /*         \/         */
  "",
  "",
  "",
  "",
  "",
  "memory allocation failed%s.",
  "%sobject creation failed.",
  "maximum input size exceeded.",
  "invalid option -%c.",
  "invalid object%s.",
  "index out of bounds%s.",
  "loading of file failed (%s).",
  "file not found (%s).",
  "invalid pointer found.",
  "normalized information distance requires two inputs."
};

/**
 * flott status information message look up table
 */
const char* flott_msg_lut_G[FLOTT_MAX_MESSAGE_CODES] =
{
    "",
    "option '%c' (%d) with '%s'"
};
