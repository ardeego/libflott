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

#include "flott_config.h"

#if defined (FLOTT_MATLAB_NTI_DIST) || defined (FLOTT_MATLAB_NTC_DIST)

#include <stdlib.h>
#include "flott.h"
#include "flott_matlab.h"
#include "mex.h"

void flott_matlab_err_handler (flott_object *op, int *propagate)
{
  *propagate = false;
  mexErrMsgTxt (op->status.message);
}

void flott_matlab_msg_handler (flott_object *op, flott_vlevel vlevel,
                               int *propagate)
{
  *propagate = false;
}

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    flott_object *op;
    flott_source *source;
    double *ret_mat;
    mwSize  length_a, length_b;

    /* check for proper number of arguments. */
    if (nrhs != 2)
    {
        mexErrMsgTxt ("Two inputs required.");
    }
    if (nlhs!= 1)
    {
        mexErrMsgTxt ("One output required.");
    }

    /* the input must be an array of bytes */
    length_a = mxGetN (prhs[0]);

    if (!mxIsUint8 (prhs[0]) || mxIsComplex(prhs[0]) || mxGetM (prhs[0]) != 1)
    {
        mexErrMsgTxt ("Improper input parameters: first string has to be a "
                      "one dimensional row vector of uint8.");
    }

    length_b = mxGetN(prhs[1]);
    if (!mxIsUint8 (prhs[1]) || mxIsComplex (prhs[1]) || mxGetM (prhs[1]) != 1)
    {
        mexErrMsgTxt ("Improper input parameters: second string has to be a "
                      "one dimensional row vector of uint8.");
    }
    plhs[0] = mxCreateDoubleMatrix(1,1, mxREAL);
    ret_mat = (double *) mxGetPr (plhs[0]);
    *ret_mat = -1.0;

#if defined (FLOTT_MATLAB_NTI_DIST)
    op = flott_create_instance (2);
#else
    op = flott_create_instance (3);
#endif
    if (op != NULL)
      {
        op->verbosity_level = FLOTT_VL_QUIET;
        op->handler.error = &flott_matlab_err_handler;
        op->handler.message = &flott_matlab_msg_handler;

        source = &(op->input.source[0]);
        source->length = length_a;
        source->storage_type = FLOTT_DEV_MEM;
        source->data.bytes = (char *) mxGetPr (prhs[0]);

        source = &(op->input.source[1]);
        source->length = length_b;
        source->storage_type = FLOTT_DEV_MEM;
        source->data.bytes = (char *) mxGetPr (prhs[1]);

        /* calculate normalized t-information / t-complexity distance */
#if defined (FLOTT_MATLAB_NTI_DIST)
        flott_nti_dist (op, ret_mat);
#else
        flott_ntc_dist (op, ret_mat);
#endif
        /* clean up */
        flott_destroy (op);
      }
    else
      {
        mexErrMsgTxt ("Unable to create flott object.");
      }
}
#endif /* FLOTT_MATLAB_NTI_DIST || FLOTT_MATLAB_NTC_DIST */
