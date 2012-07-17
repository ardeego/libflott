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

#ifndef _FLOTT_NID_H_
#define _FLOTT_NID_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct flott_user_stop_sequence flott_user_stop_sequence;

struct flott_user_stop_sequence
{
  flott_uint offset;
  double t_complexity;
};

int flott_nti_dist (flott_object *op, double *nti_dist);
int flott_ntc_dist (flott_object *op, double *ntc_dist);

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_NID_H_ */
