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
#include <math.h>
#include "flott.h"
#include "flott_nid.h"

/**
 * external prototype definitions
 */
extern void flott_t_transform_simple (flott_object *op);
extern void flott_t_transform_callback (flott_object *op);

double
flott_nid (double ab, double a, double b)
{
  double ret_val = -1.0;

  double max = flott_max_M (a, b);
  double min = flott_min_M (a, b);

  if (max > 0.0)
  {
     ret_val = (ab - min) / max;
  }
  /* printf("a %.2f, b %.2f, ab %.2f \n", x,  b, ab); */

  return ret_val;
}

int
flott_nti_dist (flott_object *op, double *nti_dist)
{
  int ret_val;
  double t_information_a;
  double t_information_b;
  double t_information_ab;

  *nti_dist = -1.0;
  ret_val = flott_initialize (op);

  if (ret_val == FLOTT_SUCCESS
      && op->input.sequence.length == 2)
    {
      op->input.append_termchar = true;
      op->handler.progress = NULL;

      flott_sequence tmp_sequence;
      size_t index;

      /* get t-information for file/memory location 'a' concatenated with 'b' */
      flott_t_transform_simple (op);
      t_information_ab = op->result.t_information;

      /* reuse already allocated token list memory */
      tmp_sequence = op->input.sequence;
      op->input.sequence.member = &index;
      op->input.sequence.length = 1;

      /* get t-information for file/memory location 'a' */
      index = tmp_sequence.member[0];
      ret_val = flott_initialize (op);
      if (ret_val == FLOTT_SUCCESS)
        {
          flott_t_transform_simple (op);
          t_information_a = op->result.t_information;

          /* get t-information for file/memory location 'b' */
          index = tmp_sequence.member[1];
          ret_val = flott_initialize (op);
          if (ret_val == FLOTT_SUCCESS)
            {
              flott_t_transform_simple (op);
              t_information_b = op->result.t_information;
            }
        }

      op->input.sequence = tmp_sequence;

      if (ret_val == FLOTT_SUCCESS)
      {
        *nti_dist = flott_nid (t_information_ab, t_information_a, t_information_b);
      }
    }
  else
    {
      ret_val = flott_set_status (op, FLOTT_ERR_NID_NUM_INPUTS, FLOTT_VL_FATAL);
    }

  return ret_val;
}

void
flott_ntc_dist_step (flott_object *op, const flott_uint level,
                   const size_t cf_value, const size_t cp_start_offset,
                   const size_t cp_length, const size_t joined_cp_length,
                   const double t_complexity, int *terminate)
{
  flott_user_stop_sequence *stop_sequence =
                            (flott_user_stop_sequence *) (op->user);
  if (cp_start_offset <= stop_sequence->offset)
    {
      stop_sequence->t_complexity = t_complexity;// - log(cf_value + 1) / log(2.0);
      op->handler.step = NULL;
    }
}

int
flott_ntc_dist (flott_object *op, double *ntc_dist)
{
  int ret_val;
  double t_complexity_a;
  double t_complexity_b;
  double t_complexity_agb;
  double t_complexity_bga;
  flott_user_stop_sequence stop_sequence;
  void *user = op->user;

  *ntc_dist = -1.0;

  if (op->input.count != 3)
    {
      ret_val = flott_set_status (op, FLOTT_ERR_NID_NUM_INPUTS, FLOTT_VL_FATAL);
    }
  else
    {
      op->input.append_termchar = true;
      op->handler.progress = NULL;

      /* initialize third input source to stop symbol */
      op->input.source[2].storage_type = FLOTT_DEV_STOP_SYMBOL;
      op->input.source[2].length = 1;

      op->input.sequence.deallocate = true; ///< auto deallocation
      op->input.sequence.member =
          (size_t *) malloc (op->input.count * sizeof (size_t));
      if (op->input.sequence.member != NULL)
        {
          op->input.sequence.length = op->input.count;
          op->input.sequence.member[0] = 1;
          op->input.sequence.member[1] = 2;
          op->input.sequence.member[2] = 0;
          stop_sequence.offset = op->input.source[0].length;
          if ((ret_val = flott_initialize (op)) == FLOTT_SUCCESS)
            {
              op->user = (void *) &stop_sequence;
              op->handler.step = &flott_ntc_dist_step;
              flott_t_transform_callback (op);
              t_complexity_a = stop_sequence.t_complexity;
              t_complexity_bga = op->result.t_complexity - t_complexity_a;
            }

          op->input.sequence.member[0] = 0;
          op->input.sequence.member[2] = 1;
          stop_sequence.offset = op->input.source[1].length;
          op->user = user;
          if ((ret_val = flott_initialize (op)) == FLOTT_SUCCESS)
            {
              op->user = (void*) &stop_sequence;
              op->handler.step = &flott_ntc_dist_step;
              flott_t_transform_callback (op);
              t_complexity_b = stop_sequence.t_complexity;
              t_complexity_agb = op->result.t_complexity - t_complexity_b;

              *ntc_dist = flott_max_M(t_complexity_agb, t_complexity_bga)
                          / flott_max_M(t_complexity_a, t_complexity_b);
            }
        }
      else
        {
          ret_val = flott_set_status (op, FLOTT_ERR_MALLOC_FLOTT,
                                      FLOTT_VL_FATAL, " (sequence list)");
        }
    }
  op->user = user;
  return ret_val;
}

