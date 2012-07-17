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

#ifndef _FLOTT_CONSOLE_H_
#define _FLOTT_CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "flott_config.h"

typedef struct flott_getopt_object flott_getopt_object;

struct flott_getopt_object
{
  char **argv;
  int argc;
  char *tpl;

  char *optarg;
  int opterr;
  int optopt;
  int optind;
  int optoff;
};

flott_getopt_object * flott_init_options (flott_getopt_object *op, char *tpl,
                                          char **argv, int argc);
int flott_get_options (flott_getopt_object *op);

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_CONSOLE_H_ */
