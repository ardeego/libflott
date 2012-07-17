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

#include <string.h>
#include "flott.h"
#include "flott_output.h"
#include "flott_math.h"
#include "flott_util.h"

#define FLOTT_OUT_MIN_COLSZ     6
#define FLOTT_OUT_SDBL_DIGITS   2
#define FLOTT_OUT_PRETTY_PAD    1

#define FLOTT_OUT_ORD_SZ       11
#define FLOTT_OUT_DBL_ORD       5
#define FLOTT_OUT_SDBL_ORD      7
#define FLOTT_OUT_CP_ORD        9
#define FLOTT_OUT_T_NID_ORD    FLOTT_OUT_ORD_SZ - 1

/**
 * function macros (indicated by '_M' suffix)
 */
#define flott_col_printf_M(option, format, value) \
    if (flott_bitset_M (options, option)) \
    { \
      fprintf (output_handle, format, column_separator, value); \
      *column_separator = output->column_separator; \
    }

#define flott_col_sprintf_M(option, format, value) \
    if (flott_bitset_M (options, option)) \
    { \
      i += sprintf (&line_buffer[i], (format), column_separator, value); \
      *column_separator = output->column_separator; \
    }


/**
 * global constants (indicated by '_G' suffix)
 */
const static flott_output_options flott_col_ordinal_G[FLOTT_OUT_ORD_SZ] =
{
  FLOTT_OUT_INPUT_OFFSET,         ///< ordinal:  0 (integer)
  FLOTT_OUT_T_AUG_LEVEL,
  FLOTT_OUT_CF,
  FLOTT_OUT_CP_OFFSET,
  FLOTT_OUT_CP_LENGTH,
  FLOTT_OUT_T_COMPLEXITY,         ///< ordinal:  5 (double)
  FLOTT_OUT_T_INFORMATION,
  FLOTT_OUT_AVE_T_ENTROPY,        ///< ordinal:  7 (short double)
  FLOTT_OUT_INST_T_ENTROPY,
  FLOTT_OUT_CP_STRING             ///< ordinal:  9 (data/string)
};

const static char* flott_col_label_G[FLOTT_OUT_ORD_SZ] =
{
  "x",       ///< input offset
  "n",       ///< level
  "k",       ///< copy factor
  "p-{o}",   ///< copy pattern offset
  "p-{l}",   ///< copy pattern length
  "t-{c}",   ///< t-complexity
  "t-{i}",   ///< t-information
  "t-{e}",   ///< average t-entropy rate
  "t-{r}",   ///< instantaneous t-entropy rate
  "p",        ///< copy pattern string
  "t-{nid}"  ///< normalized t-information distance
};

int
flott_set_output_device (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;
  flott_user_output *output = (flott_user_output *) (op->user);
  /* set output device */
  switch (output->storage_type)
    {
      case FLOTT_DEV_STDOUT : output->handle = stdout; break;
      case FLOTT_DEV_FILE :
        {
          output->handle = fopen (output->path, "wb");
          if (output->handle == NULL)
            {
              /* we could not create output file */
              /*TODO: throw proper error message */
              ret_val = flott_set_status (op, FLOTT_ERROR, FLOTT_VL_FATAL);
            }
        }
        break;
      default : /* we got an invalid output option */
        {
          /*TODO: throw proper error message */
          ret_val = flott_set_status (op, FLOTT_ERROR, FLOTT_VL_FATAL);
        }
        break;
    }

  return ret_val;
}

int
flott_output_initialize (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;

  int int_sz = 0;
  int double_sz = 0;
  int sdouble_sz = 0;
  int line_length, i;
  char column_separator[2] = "";
  char column_format[FLOTT_COLFORM_BUFSZ];
  flott_private *private = &(op->private);
  flott_user_output *output = (flott_user_output *) (op->user);

  output->previous_t_information = 0.0;
  output->previous_input_offset = op->input.length - 1;
  output->scale_factor = 1.0;
  if (flott_bitset_M (output->options, FLOTT_OUT_UNITS_BITS))
    {
      output->scale_factor = private->ln2;
    }

  *(output->column_header) = '\0';
  *(output->line_buffer) = '\0';

  /* set column separating charter */
  switch (output->options
         & (FLOTT_OUT_PRETTY | FLOTT_OUT_CSV | FLOTT_OUT_TAB))
    {
      case FLOTT_OUT_CSV : output->column_separator = ','; break;
      case FLOTT_OUT_TAB : output->column_separator = '\t'; break;
      case FLOTT_OUT_PRETTY :
      default :
        {
          output->options |= FLOTT_OUT_PRETTY;
          output->column_separator = ' ';

          /* determine appropriate column widths for numeric column values */
          int_sz = flott_get_digit_count (op->input.length);
          int_sz += FLOTT_OUT_PRETTY_PAD;              ///< extra padding

          double_sz = int_sz + output->precision + 1;  ///< +1: decimal dot

          sdouble_sz = output->precision + FLOTT_OUT_SDBL_DIGITS + 1;
          sdouble_sz += FLOTT_OUT_PRETTY_PAD;          ///< extra padding

          /* ensure minimum column widths */
          int_sz = flott_max_M (int_sz, FLOTT_OUT_MIN_COLSZ);
          double_sz = flott_max_M (double_sz, FLOTT_OUT_MIN_COLSZ);
          sdouble_sz = flott_max_M (sdouble_sz, FLOTT_OUT_MIN_COLSZ);
        }
        break;
    }

  /* if normalized t-information distance option is set write column header
   * and double format for 't-{nid}' and return early */
  if (flott_bitset_M (output->options, FLOTT_OUT_NTI_DIST)
      || flott_bitset_M (output->options, FLOTT_OUT_NTC_DIST))
    {
      /*TODO boundary checking */
      double_sz = 0;
      strcpy (column_format, "%s");

      if (flott_bitset_M (output->options, FLOTT_OUT_PRETTY))
        {
          double_sz = output->precision + 2; ///< +2: (one digit + decimal dot)
          double_sz = flott_max_M (double_sz, FLOTT_OUT_MIN_COLSZ + 2);
          sprintf (column_format, "%%%ds", double_sz);
        }

      sprintf (output->column_header, column_format,
               flott_col_label_G[FLOTT_OUT_T_NID_ORD]);
      sprintf (output->basic_double, "%%%d.%df", double_sz,
               output->precision);

      op->handler.init = NULL; ///<< init output format only once

      return ret_val;
    }

  /* ensure that fixed buffer sizes are not exceeded -- then it is
   * safe to use 'sprint' and 'strcpy' */
  line_length = (FLOTT_OUT_ORD_SZ
                    * ( flott_get_digit_count (double_sz)
                        + (FLOTT_OUT_PRETTY_PAD + 1) * FLOTT_OUT_ORD_SZ) );

  if ( (flott_get_digit_count (double_sz) + 5) < FLOTT_COLFORM_BUFSZ
      && line_length < FLOTT_LINE_BUFSZ )
    {
      /* setting generic number printing formats */
      sprintf (output->basic_int, "%%s%%%d%s", int_sz, FLOTT_PRINTF_T_SIZE_T);
      sprintf (output->basic_double, "%%s%%%d.%df", double_sz,
               output->precision);
      sprintf (output->short_double, "%%s%%%d.%df", sdouble_sz,
               output->precision);

      /* generate column header line */
      if (flott_bitset_M (output->options, FLOTT_OUT_HEADERS))
        {
          line_length = 0;
          /* set int header format */
          sprintf (column_format, "%%s%%%ds", int_sz);
          for (i = 0; i < FLOTT_OUT_ORD_SZ - 1; i++)
            {
              if (i >= FLOTT_OUT_CP_ORD)
                {
                  /* no padding for copy pattern */
                  if (flott_bitset_M (output->options, FLOTT_OUT_PRETTY))
                    {
                      strcpy (column_format, "%s%2s");
                    }
                  else
                    {
                      strcpy (column_format, "%s%s");
                    }
                }
              else if (i >= FLOTT_OUT_DBL_ORD && i < FLOTT_OUT_SDBL_ORD)
                {
                  /* set double padding format */
                  sprintf (column_format, "%%s%%%ds", double_sz);
                }
              else if (i >= FLOTT_OUT_SDBL_ORD)
                {
                  /* set short padding format */
                  sprintf (column_format, "%%s%%%ds", sdouble_sz);
                }

              if (flott_bitset_M (output->options, flott_col_ordinal_G[i]))
                {
                  line_length += sprintf (&(output->column_header[line_length]),
                                          column_format, column_separator,
                                          flott_col_label_G[i]);
                  *column_separator = output->column_separator;
                }
            }
        }
    }
  else
    {
      /* TODO error out */
    }

  return ret_val;
}

void flott_output_print_headers (const flott_object *op)
{
  flott_user_output *output = (flott_user_output *) (op->user);
  if (flott_bitset_M (output->options, FLOTT_OUT_HEADERS))
    {
      if (output->column_header != NULL && output->handle != NULL)
        {
          fprintf (output->handle, "%s\n", output->column_header);
        }
    }
}

int flott_output_nti_dist (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;
  flott_user_output *output = (flott_user_output *) (op->user);
  char* basic_double = output->basic_double;
  double nti_dist;

  /* calculate normalized t-information distance */
  if ((ret_val = flott_nti_dist (op, &nti_dist)) == FLOTT_SUCCESS)
    {
      flott_output_print_headers (op);
      fprintf (output->handle, basic_double, nti_dist);
      fprintf(output->handle, "\n");
    }

  return ret_val;
}

int flott_output_ntc_dist (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;
  flott_user_output *output = (flott_user_output *) (op->user);
  char* basic_double = output->basic_double;
  double ntc_dist;

  /* calculate normalized t-complexity distance */
  if ((ret_val = flott_ntc_dist (op, &ntc_dist)) == FLOTT_SUCCESS)
    {
      flott_output_print_headers (op);

      fprintf (output->handle, basic_double, ntc_dist);
      fprintf(output->handle, "\n");
    }

  return ret_val;
}

void flott_output_no_rate (flott_object *op)
{
  flott_user_output *output = (flott_user_output *) (op->user);
  flott_uint options = output->options;
  FILE *output_handle = output->handle;
  char* basic_int = output->basic_int;
  char* basic_double = output->basic_double;
  char* short_double = output->short_double;
  char column_separator[2] = "";

  flott_uint levels;
  double t_complexity, t_information, t_entropy;

  if (op->input.length > 0)
    {
      op->handler.step = NULL; ///< no t-augmentation step output
      flott_t_transform (op);

      /* scale results to 'nats'/'bits' */
      levels = op->result.levels;
      t_complexity = op->result.t_complexity;
      t_information = op->result.t_information / output->scale_factor;
      t_entropy = op->result.t_entropy / output->scale_factor;

      flott_output_print_headers (op);
      flott_col_printf_M (FLOTT_OUT_T_AUG_LEVEL, basic_int, (size_t) levels);
      flott_col_printf_M (FLOTT_OUT_T_COMPLEXITY, basic_double, t_complexity);
      flott_col_printf_M (FLOTT_OUT_T_INFORMATION, basic_double, t_information);
      flott_col_printf_M (FLOTT_OUT_AVE_T_ENTROPY, short_double, t_entropy);

      fprintf(output->handle, "\n");
    }
}

void
flott_output_step (flott_object *op, const flott_uint level,
                   const size_t cf_value, const size_t cp_start_offset,
                   const size_t cp_length, const size_t joined_cp_length,
                   const double t_complexity, int *terminate)
{
  flott_user_output *output = (flott_user_output *) (op->user);
  flott_uint options = output->options;
  FILE *output_handle = output->handle;
  size_t i = 0;
  size_t input_offset;
  char* line_buffer = output->line_buffer;
  char* basic_int = output->basic_int;
  char* basic_double = output->basic_double;
  char* short_double = output->short_double;
  char column_separator[2] = "";
  double t_information;
  double t_entropy;
  double t_inst_entropy;

  /* check if t-information has to be calculated */
  if ( options & (FLOTT_OUT_T_INFORMATION | FLOTT_OUT_AVE_T_ENTROPY
                                          | FLOTT_OUT_INST_T_ENTROPY) )
  {
      t_information =
          flott_get_t_information (t_complexity) / output->scale_factor;

      t_entropy =
          t_information / ( (op->private.token_list.length - cp_start_offset)
                           + (joined_cp_length - cp_length) + 1 );
      t_inst_entropy =
          (t_information - output->previous_t_information) / joined_cp_length;

      output->previous_t_information = t_information;
  }

  /* check if output needs to be interpolated (i.e. fill the gaps) */
  if (flott_bitset_M (output->options, FLOTT_OUT_INPUT_OFFSET))
  {
     /* buffer boundary check from 'output_init' applies here too */
    flott_col_sprintf_M (FLOTT_OUT_INPUT_OFFSET, "%s%s", basic_int);
    flott_col_sprintf_M (FLOTT_OUT_T_AUG_LEVEL, basic_int, (size_t) level);
    flott_col_sprintf_M (FLOTT_OUT_CF, basic_int, cf_value);
    flott_col_sprintf_M (FLOTT_OUT_CP_OFFSET, basic_int, cp_start_offset);
    flott_col_sprintf_M (FLOTT_OUT_CP_LENGTH, basic_int, cp_length);
    flott_col_sprintf_M (FLOTT_OUT_T_COMPLEXITY, basic_double, t_complexity);
    flott_col_sprintf_M (FLOTT_OUT_T_INFORMATION, basic_double, t_information);
    flott_col_sprintf_M (FLOTT_OUT_AVE_T_ENTROPY, short_double, t_entropy);
    flott_col_sprintf_M (FLOTT_OUT_INST_T_ENTROPY, short_double, t_inst_entropy);

    *column_separator = '\0';
    input_offset = (cp_start_offset + cp_length - joined_cp_length);
    for (i = output->previous_input_offset - 1; i > input_offset; i--)
      {
        fprintf (output_handle, line_buffer, column_separator, i);
        if (flott_bitset_M (options, FLOTT_OUT_CP_STRING))
          {
            //*column_separator = output->column_separator;
            if (flott_bitset_M (options, FLOTT_OUT_PRETTY))
              {
                fprintf (output_handle, "%2c.", output->column_separator);
              }
            else
              {
                fprintf (output_handle, "%c", output->column_separator);
              }
          }
        fprintf (output_handle, "\n");
      }
    output->previous_input_offset = input_offset;
  }

  flott_col_printf_M (FLOTT_OUT_INPUT_OFFSET, basic_int, input_offset);
  flott_col_printf_M (FLOTT_OUT_T_AUG_LEVEL, basic_int, (size_t) level);
  flott_col_printf_M (FLOTT_OUT_CF, basic_int, cf_value);
  flott_col_printf_M (FLOTT_OUT_CP_OFFSET, basic_int, cp_start_offset);
  flott_col_printf_M (FLOTT_OUT_CP_LENGTH, basic_int, cp_length);
  flott_col_printf_M (FLOTT_OUT_T_COMPLEXITY, basic_double, t_complexity);
  flott_col_printf_M (FLOTT_OUT_T_INFORMATION, basic_double, t_information);
  flott_col_printf_M (FLOTT_OUT_AVE_T_ENTROPY, short_double, t_entropy);
  flott_col_printf_M (FLOTT_OUT_INST_T_ENTROPY, short_double, t_inst_entropy);

  if (flott_bitset_M (options, FLOTT_OUT_CP_STRING))
    {
      if (flott_bitset_M (options, FLOTT_OUT_PRETTY))
        {
          fprintf (output_handle, "%2s", column_separator);
        }
      else
        {
          fprintf (output_handle, "%s", column_separator);
        }

      /* TODO: check return value & set terminate on error */
      *terminate = false;
      flott_input_write ((flott_object *) op,
                         cp_start_offset,
                         cp_length,
                         output_handle);
    }
  fprintf(output_handle, "\n");
}

int flott_output (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;
  flott_user_output *output = (flott_user_output *) (op->user);
  size_t member;

  if (flott_bitset_M (output->options, FLOTT_OUT_CONCAT_INPUT))
    {
      ret_val = flott_initialize (op);
      if (output->options & FLOTT_OUT_STEP)
        {
          op->handler.progress = NULL; ///< no progress bar;
          flott_output_print_headers (op);
          flott_t_transform (op);
        }
      else /* simple output (no t-augmentation step output ) */
        {
          flott_output_no_rate (op);
        }
    }
  else
    {
      op->input.sequence.deallocate = false;
      op->input.sequence.length = 1;
      op->input.sequence.member = &member;

      for (member = 0; member < op->input.count; member++)
        {
          if ((ret_val = flott_initialize (op)) != FLOTT_SUCCESS)
            {
              break;
            }

          if ( (output->options & FLOTT_OUT_PRETTY)
               &&  *(output->column_header) != '\0' )
            {
              fprintf (output->handle,
                       "input #%" FLOTT_PRINTF_T_SIZE_T ":\n" , member + 1);
            }

          if (output->options & FLOTT_OUT_STEP)
            {
              flott_output_print_headers (op);
              flott_t_transform (op); /* TODO: error check here */
            }
          else /* simple output (no t-augmentation step output) */
            {
              flott_output_no_rate (op);
            }

          if (member != op->input.count) fprintf (output->handle, "\n");
        }
    }

  return ret_val;
}

void
flott_output_destroy (const flott_object *op)
{
  flott_user_output *output = (flott_user_output *) (op->user);
  if (output != NULL && output->handle != NULL)
     {
       if (output->handle != stdout)
         {
           fclose (output->handle);
           output->handle = NULL;
         }
     }
}

#ifdef _MSC_VER

#include <windows.h>
/* this should have been easy bill, but hey i guess not so... */
void flott_output_progress_bar (const flott_object *op, const float ratio)
{
  int i;
  int bar_length = ((flott_user_output *) (op->user))->progress_bar_length;
  int bar_done = (int) (ratio * bar_length);
  CONSOLE_SCREEN_BUFFER_INFO sb_info;
  HANDLE con_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  /* TODO: there should be some error checking here,
   * but hey, it's just windows, who really cares? */
  GetConsoleScreenBufferInfo(con_handle, &sb_info);

  /* print percentage done */
  fprintf (stdout, "%3d%% [", (int)(ratio * 100));

  /* print 'complete' portion of bar */
  for (i = 0; i < bar_done; i++)
     fprintf (stdout, "%c", '=');

  /* print 'incomplete' portion of bar */
  for (; i < bar_length; i++)
     fprintf(stdout, "%c", ' ');

  fprintf (stdout, "%s", "]");

  SetConsoleCursorPosition(con_handle, sb_info.dwCursorPosition);

  /* make progress bar disappear when we are done */
  if (ratio == 1.0)
    {
      for (i = 0; i < bar_length + 7; /* 7 non-bar chars */ i++)
         fprintf(stdout, "%c", ' ');

      SetConsoleCursorPosition(con_handle, sb_info.dwCursorPosition);
    }
  fflush (stdout);
}

#else /* _MSC_VER */

void flott_output_progress_bar (const flott_object *op, const float ratio)
{
  int i;
  int bar_length = ((flott_user_output *) (op->user))->progress_bar_length;
  int bar_done = (int) (ratio * bar_length);

  /* print percentage done */
  printf("%3d%% [", (int)(ratio * 100));

  /* print 'complete' portion of bar */
  for (i = 0; i < bar_done; i++)
     printf("=");

  /* print 'incomplete' portion of bar */
  for (; i < bar_length; i++)
     printf(" ");

  /* clear the previous line on next write */
  printf("]\n\033[F\033[J");
}

#endif /* _MSC_VER */
