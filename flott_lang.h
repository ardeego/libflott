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

#ifndef _FLOTT_LANG_H_
#define _FLOTT_LANG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FLOTT_MAX_MESSAGE_CODES   2

/**
 * flott message codes
 */
enum flott_msg_codes
{
  FLOTT_CUSTOM_MSG            =  0,
  FLOTT_CMD_OPTION_PARAM      =  1
};

/**
 * static global string resources (global variables indicated by '_G' suffix)
 */
extern const char* flott_errmsg_lut_G[];
extern const char* flott_msg_lut_G[];
extern const char* flott_msg_help_G;

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_LANG_H_ */
