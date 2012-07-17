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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "flott.h"
#include "flott_lang.h"
#include "flott_term.h"

flott_getopt_object *
flott_init_options (flott_getopt_object *op, char *tpl, char **argv, int argc)
{
  /* initialize options structure */
  if (op != NULL)
    {
      op->argv = argv;
      op->argc = argc;
      op->tpl = tpl;

      op->optind = 1;
      op->opterr = 0;
      op->optoff = 0;
    }

  return op;
}

int
flott_get_options (flott_getopt_object *op)
{
  int letter = -1;
  char **argv = op->argv;
  int argc = op->argc;
  char* tpl_pointer;

  op->optarg = NULL;
  if(op->optind >= op->argc || argv[op->optind] == NULL) /* we reached the end */
    {
      letter  = -1;
    }
  else  /* check for start of an option */
    {
      if (argv[op->optind][op->optoff] == '-') /* check for start of an option */
        {
          (op->optoff)++;
        }
      else
        {
          if (op->optoff == 0)
            {
              letter = 1;
              op->optarg = &(argv[op->optind][0]);
              (op->optind)++;
              return letter;
            }
        }

      tpl_pointer = NULL;
      letter = argv[op->optind][(op->optoff)++];

      /* check if the option letter is in the options template */
      if (op->tpl[0] == '-')
        {
          tpl_pointer = strchr ((op->tpl) + 1, letter);
        }
      else
        {
          tpl_pointer = strchr ((op->tpl), letter);
        }

      /* option is unknown */
      if (tpl_pointer == NULL)
        {
          if (op->opterr)
            {
              fprintf(stderr, "%s: invalid option  -%c\n", argv[0], letter);
            }

          op->optopt = letter;
          letter = '?';

          if (argv[op->optind][op->optoff] == '\0')
            {
              (op->optind)++;
              op->optoff = 0;
            }
        }
      else
        {
          /* check if the option requires an argument */
          if (tpl_pointer[1] == ':')
            {
              /* required, concatenated argument */
              if (argv[op->optind][op->optoff] != '\0')
                {
                  op->optarg = &(argv[op->optind][op->optoff]);
                  (op->optind)++;
                  op->optoff = 0;
                }
              else /* separated argument */
                {
                  /* TODO: ugly, flawed,
                   * so we may need to do some re-writing here at some point */
                  if (++(op->optind) < argc
                      && !(argv[op->optind][0] == '-'
                          && strchr ((op->tpl), (int)(argv[op->optind][1])) != NULL)
                     )
                    {
                      op->optarg = &(argv[op->optind][0]);
                      (op->optind)++;
                    }

                  op->optoff = 0;
                }

              /* optional argument */
              if (op->optarg == NULL && tpl_pointer[2] != ':')
                {
                  op->optopt = letter;
                  letter = '?';

                  if (op->opterr)
                    {
                      fprintf (stderr,
                               "%s: option requires an argument -%c\n",
                               argv[0], op->optopt);
                    }
                }
            }
          else
            {
              if (argv[op->optind][op->optoff] == '\0')
                {
                  (op->optind)++;
                  op->optoff = 0;
                }
            }
        }
    }

  return letter;
}
