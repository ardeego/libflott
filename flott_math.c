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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXIT 100
#define EULER 0.5772156649
#define FPMIN 5.0e-308
#define EPS 5.0e-16

/**
 * this function is derived from Mark Titchener's original tcalc implementation
 * and is based is used to numerically evaluate the inverse logarithmic integral
 * converting t-complexity into t-information.
 *
 * The implementation is based on methods provided by
 * W. H. Press et.al., 'Numerical Recipes in C', Cambridge University Press, 1992.
 */
double
convert (double x)
{
  int k;
  double fact, prev, sum, term;

  if (x <= 0.0)
      printf ("Bad argument in convert\n");
  if (x < FPMIN) return log (x) + EULER;
  if (x <= -log (EPS))
    {
      sum=0.0;
      fact=1.0;
      for (k=1; k <= MAXIT; k++)
        {
          fact *= x / k;
          term = fact / k;
          sum += term;
          if (term < EPS * sum)
              break;
        }
      if (k > MAXIT)
          printf ("series failed in convert\n");
      return sum + log (x) + EULER;
    }
  else
    {
      sum = 0.0;
      term = 1.0;
      for (k = 1; k <= MAXIT; k++)
        {
          prev = term;
          term *= k / x;
          if (term < EPS) break;
          if (term < prev) sum += term;
          else
            {
              sum -= prev;
              break;
            }
        }
      return exp (x) * (1.0 + sum) / x;
    }
}

/**
 * this function is derived from Mark Titchener's original tcalc implementation
 * and is based is used to numerically evaluate the inverse logarithmic integral
 * converting t-complexity into t-information.
 */
double
flott_get_t_information (double t_complexity)
{
  double lb, ub, tmp1, tmp2, g1;
  lb = 1.0 + EPS;
  if (t_complexity < convert (log (lb)))
    {
      return 0;
    }
  ub = 2.0;
  while (convert (log (ub)) < t_complexity)
    {
      lb = ub;
      ub *= 2;
    }
  tmp1 = convert (log (lb));
  tmp2 = convert (log (ub));
  g1 = 1 / log (lb);

  while (tmp2 - tmp1 > EPS)
    {
      ub = (t_complexity - tmp1) * (ub - lb) / (tmp2 - tmp1) + lb;
      tmp2 = convert (log (ub));
      lb += (t_complexity - tmp1) / g1;
      tmp1 = convert (log (lb));
      g1 = 1 / log (lb);
    }

  return (ub);
}
