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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "flott.h"
#include "flott_term.h"
#include "flott_output.h"
#include "flott_util.h"

void
help (void)
{
  fprintf (stdout, "%s", flott_msg_help_G);
}

int
set_input_sources (flott_object* op, char** argv, int argc,
                   bool buffer_input_flag)
{
  int ret_val = FLOTT_SUCCESS;

  flott_getopt_object options;
  flott_source *source;
  size_t input_count = 0;

  flott_init_options (&options, "-S:I:", argv, argc);
  options.opterr = 0;

  op->input.source =
      (flott_source *) malloc (sizeof (flott_source) * (op->input.count));

  if (op->input.source != NULL)
    {
      while (true)
        {
          int letter = flott_get_options (&options);
          if (letter == -1) break;

          switch (letter)
            {
              case 'S':
                {
                  if (options.optarg != NULL)
                    {
                      source = &(op->input.source[input_count++]);
                      source->storage_type = FLOTT_DEV_MEM;
                      source->length = strlen(options.optarg);
                      source->data.bytes = options.optarg;
                    }
                  else
                    {
                      ret_val = flott_set_status (op, FLOTT_ERR_NULL_POINTER,
                                                  FLOTT_VL_FATAL);
                    }
                }
                break;
              case 'I':
                {
                  if (options.optarg != NULL
                      && flott_file_exists (options.optarg))
                    {
                      source = &(op->input.source[input_count++]);
                      source->storage_type = FLOTT_DEV_FILE;
                      if (buffer_input_flag == true)
                        {
                          source->storage_type = FLOTT_DEV_FILE_TO_MEM;
                        }
                      source->length = flott_get_file_size(options.optarg);
                      source->path = options.optarg;
                    }
                  else
                    {
                      ret_val = flott_set_status (op, FLOTT_ERR_FILE_NOT_FOUND,
                          FLOTT_VL_FATAL, options.optarg);
                    }
                }
                break;
              /* we don't care about other options, as this is the second run */
              default: break;
            }
        }
    }
  else
    {
      ret_val = flott_set_status (op, FLOTT_ERR_MALLOC_FLOTT,
          FLOTT_VL_FATAL, " (source list)");
    }

  return ret_val;
}

void
set_symbol_type (flott_symbol_type *symbol_type, char* optarg)
{
  *symbol_type = FLOTT_SYMBOL_BYTE;
  if (optarg != NULL)
    {
      if (atoi(optarg) == 1) *symbol_type = FLOTT_SYMBOL_BIT;
    }
}

int
set_int_argument(const char* argument, int min, int max, int def_val)
{
  int ret_val = def_val;

  /* supply the default value if no argument its given*/
  if(argument != NULL)
    {

      /* check if we actually got an integer */
      if (sscanf(argument, "%d", &ret_val) != 1)
        {
          ret_val = def_val;
        }
      else
        {
          /* check for proper rage of integer */
          if (max < ret_val || ret_val < min)
            {
              ret_val = def_val;;
            }
        }
    }

  return ret_val;
}

void
set_column_format (flott_output_options *options, char* optarg)
{
  if (optarg != NULL && strncmp("=tab", optarg, 4) == 0)
    {
      *options |= FLOTT_OUT_TAB;
    }
  else if (optarg != NULL && strncmp("=csv", optarg, 4) == 0)
    {
      *options |= FLOTT_OUT_CSV;
    }
  else
    {
      *options |= FLOTT_OUT_PRETTY;
    }
}

void
set_output_units (flott_output_options *options, char* optarg)
{
  if (optarg != NULL && strncmp("=nats", optarg, 5) == 0)
    {
      *options &= ~FLOTT_OUT_UNITS_BITS;
    }
  else
    {
      *options |= FLOTT_OUT_UNITS_BITS;
    }
}

int parse_command_line (flott_object *op,
                        flott_user_output *output,
                        int argc, char *argv[])
{
  int ret_val = FLOTT_SUCCESS;

  int input_count = 0;
  int letter;
  bool quiet_flag = false;
  bool buffer_input_flag = false;
  flott_getopt_object options;

  /* set allowed command line switches and parse input arguments */
  flott_init_options (&options, "-hqv:dDcierxnkpolI:S:b:jzmo:O:F:u:g:L",
                      argv, argc);

  /* parse and process command line arguments */
  while (ret_val == FLOTT_SUCCESS)
   {
     letter = flott_get_options (&options);
     if (letter == -1) break;
     switch (letter)
       {
         case 'h': help (); flott_destroy (op); exit(0);
         case 'v': {
                     if (quiet_flag == false)
                       {
                         op->verbosity_level =
                             set_int_argument(options.optarg, FLOTT_VL_QUIET,
                                              FLOTT_VL_DEBUG,
                                              FLOTT_VL_FATAL /* default */);
                       }
                   }
                   break;
         case 'q': {
                     quiet_flag = true;
                     op->verbosity_level =
                         set_int_argument("0" /* FLOTT_VL_QUIET */,
                                          FLOTT_VL_QUIET, FLOTT_VL_DEBUG,
                                          FLOTT_VL_QUIET /* default */);
                   }
                   break;
         case 'S': if (options.optarg != NULL) input_count++;
                   break;
         case 'I': {
                     if (options.optarg != NULL
                         && flott_file_exists (options.optarg))
                       {
                         input_count++;
                       }
                   }
                   break;
         case 'd': output->options |= FLOTT_OUT_NTI_DIST;
                   break;
         case 'D': output->options |= FLOTT_OUT_NTC_DIST;
                   op->input.append_termchar = true;
                   break;
         case 'c': output->options |= FLOTT_OUT_T_COMPLEXITY;
                   break;
         case 'i': output->options |= FLOTT_OUT_T_INFORMATION;
                   break;
         case 'e': output->options |= FLOTT_OUT_AVE_T_ENTROPY;
                   break;
         case 'r': output->options |= FLOTT_OUT_INST_T_ENTROPY;
                   break;
         case 'x': output->options |= FLOTT_OUT_INPUT_OFFSET;
                   break;
         case 'n': output->options |= FLOTT_OUT_T_AUG_LEVEL;
                   break;
         case 'k': output->options |= FLOTT_OUT_CF;
                   break;
         case 'p': output->options |= FLOTT_OUT_CP_STRING ;
                   break;
         case 'o': output->options |= FLOTT_OUT_CP_OFFSET;
                   break;
         case 'l': output->options |= FLOTT_OUT_CP_LENGTH;
                   break;
         case 'b': set_symbol_type (&(op->input.symbol_type), options.optarg);
                   break;
         case 'j': output->options |= FLOTT_OUT_CONCAT_INPUT;
                   break;
         case 'z': op->input.append_termchar = true;
                   break;
         case 'm': buffer_input_flag = true;
                   break;
         case 'O': {
                     output->storage_type = FLOTT_DEV_FILE;
                     output->path = options.optarg;
                   }
                   break;
         case 'F': set_column_format (&(output->options), options.optarg);
                   break;
         case 'u': set_output_units (&(output->options), options.optarg);
                   break;
         case 'g': output->precision = set_int_argument(
                    options.optarg, 0, 300, 2 /* default */);
                   break;
         case 'L': output->options |= FLOTT_OUT_HEADERS;
                   break;
         case  1 : break;
         case '?': {
                     /* TODO differentiate: no parameter error */
                     ret_val = flott_set_status (op, FLOTT_ERR_INVALID_OPT,
                                                 FLOTT_VL_FATAL, options.optopt);
                     help ();
                     return ret_val;
                   }
                   break;
         default: {
                    /* we should never get here */
                    ret_val = flott_set_status (op, FLOTT_ERROR, FLOTT_VL_FATAL);
                    return ret_val;
                  }
                  break;
        }

     /* debug message */
     flott_set_status (op, FLOTT_CMD_OPTION_PARAM, FLOTT_VL_DEBUG,
                       letter, letter, options.optarg);
   }

  if (flott_bitset_M(output->options, FLOTT_OUT_CP_STRING)
      && op->input.symbol_type == FLOTT_SYMBOL_BIT)
    {
      output->options &= ~FLOTT_OUT_CP_STRING;
    }

  /* now that we know how many inputs we have, load all input data */
  if (ret_val == FLOTT_SUCCESS)
    {
      if (input_count == 1) output->options |= FLOTT_OUT_CONCAT_INPUT;
      if (flott_bitset_M (output->options, FLOTT_OUT_NTC_DIST))
      {
        input_count++;
      }
      op->input.count = input_count;

      ret_val = set_input_sources (op, argv, argc, buffer_input_flag);
      if (ret_val != FLOTT_SUCCESS)
        {
          return ret_val;
        }
    }

  return ret_val;
}

#ifdef FLOTT_TERMINAL_APPLICATION
int
main (int argc, char *argv[])
{
  int ret_val = FLOTT_SUCCESS;
  flott_user_output output = {0}; ///< custom data structure used to print results

  flott_object *op = flott_create_instance (0); ///< application flott object

  if (op != NULL)
    {
      /* set default flott options */
      op->input.append_termchar = false;     ///< default: no dummy character
      op->verbosity_level = FLOTT_VL_FATAL;

      /* deallocate all dynamic memory on 'flott_destroy' */
      op->input.deallocate = true;

      /* assign pointer to custom 'user' data structure */
      op->user = &output;

      /*set default output options */
      output.options |= FLOTT_OUT_UNITS_BITS; ///< default output unit: 'bits'
      output.precision = 2;                   ///< floating point precision
      output.progress_bar_length = 48;        ///< console progress bar length
      output.storage_type = FLOTT_DEV_STDOUT; ///< default output is 'stdout'


      /* parse command line arguments and update 'options' accordingly */
      ret_val = parse_command_line (op, &output, argc, argv);
      if (ret_val != FLOTT_SUCCESS)
        {
          flott_destroy (op);
          return ret_val;
        }

      /* set custom user defined callback handlers for user functions */
      op->handler.init = &flott_output_initialize;
      op->handler.step = &flott_output_step;
      op->handler.destroy = &flott_output_destroy;

      if(op->verbosity_level != FLOTT_VL_QUIET)
        {
          op->handler.progress = &flott_output_progress_bar;
        }

      /* prepare output device for writing */
      if ((ret_val = flott_set_output_device (op)) != FLOTT_SUCCESS)
        {
          /* clean up */
          flott_destroy (op);
          return ret_val;
        }

      /* normalized t-information distance */
      if (flott_bitset_M (output.options, FLOTT_OUT_NTI_DIST))
        {
          ret_val = flott_output_nti_dist (op);
        }
      else if (flott_bitset_M (output.options, FLOTT_OUT_NTC_DIST))
        {
          ret_val = flott_output_ntc_dist (op);
        }
      else /* t-transform */
        {
          ret_val = flott_output (op);
        }

      /* clean up */
      flott_destroy (op);
    }
  else
    {
      fprintf (stderr, "Unable to create flott object.");
      ret_val = FLOTT_ERR_CREATE_OBJ;
    }

  fflush(stdout);
  fflush(stderr);

  return ret_val;
}
#endif

#ifdef MAKE_LOG_LUT
int
main (int argc, char *argv[])
{
  double log2_val;
  int i;
  printf("#define LOG2_LUT \\\n{ ");

  for(i = 0; i < 512; i++)
    {
      log2_val = atof("Inf");
      if (i != 0) log2_val = log2(i);
      printf ("{%#llxLL}, ", *(unsigned long long *)&log2_val);
      if ((i+1) % 3 == 0 && i != 0) printf("\\\n  ");
    }
  return 0;
}
#endif

