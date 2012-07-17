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

#ifndef _FLOTT_H_
#define _FLOTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "flott_platform.h"
#include "flott_config.h"
#include "flott_error.h"
#include "flott_lang.h"

/**
 * constant 'define' macros
 */
#define FLOTT_PAGE_SIZE 4096        ///< use pages of 4K size
#define FLOTT_NIL 0                 ///< terminating offset value for linked lists
#define FLOTT_UINT_MAX UINT32_MAX   ///< maximum addressable token memory units
#define FLOTT_LINE_BUFSZ 1024       ///< size of line (text) buffer
#define FLOTT_STOP_SYMBOL 256

/**
 * function macros (indicated by '_M' suffix)
 */
#define flott_get_ptr_M(base, offset) &((base)[(size_t) (offset)])
//#define flott_get_ptr_M(base, offset) (base + ((size_t) offset))
#define flott_offset_M(offset) (flott_uint) (offset)
#define flott_bitset_M(field, bit) (((field) & bit) == bit)
#define flott_min_M(a, b) (((a) < (b)) ? (a) : (b))
#define flott_max_M(a, b) (((a) > (b)) ? (a) : (b))

/**
 * data type definitions
 */
typedef uint32_t flott_uint;

typedef enum flott_vlevel flott_vlevel;
typedef enum flott_symbol_type flott_symbol_type;
typedef enum flott_storage_type flott_storage_type;
typedef enum flott_mode flott_mode;

typedef struct flott_token flott_token;
typedef struct flott_match_list flott_match_list;
typedef struct flott_token_list flott_token_list;

typedef struct flott_source flott_source;
typedef struct flott_sequence flott_sequence;
typedef struct flott_input flott_input;
typedef struct flott_result flott_result;
typedef struct flott_handler flott_handler;
typedef struct flott_status flott_status;
typedef struct flott_private flott_private;
typedef struct flott_object flott_object;

/**
 * function pointer type definitions
 */
typedef int (flott_init_handler) (flott_object *);
typedef void (flott_destroy_handler) (const flott_object *);

typedef void (flott_error_handler) (flott_object *, int *);
typedef void (flott_message_handler) (flott_object *, flott_vlevel, int *);
typedef void (flott_progress_handler) (const flott_object *, const float);
typedef void (flott_step_handler) (flott_object *, const flott_uint,
                                   const size_t, const size_t, const size_t,
                                   const size_t, const double, int *);
/**
 * verbosity levels
 */
enum flott_vlevel
{
  FLOTT_VL_QUIET    = 0,
  FLOTT_VL_FATAL    = 1,
  FLOTT_VL_INFO     = 2,
  FLOTT_VL_LOG      = 3,
  FLOTT_VL_WARN     = 4,
  FLOTT_VL_DEBUG    = 5,
};

enum flott_symbol_type
{
  FLOTT_SYMBOL_BIT      =   2,  ///< 1-bit symbol length S={0,1}
  FLOTT_SYMBOL_BYTE_DNA =   4,  ///< 8-bit symbol (DNA)  S={A, C, G, T}
  FLOTT_SYMBOL_BYTE     = 256,  ///< 8-bit symbol length S={0, .., 255}
};

enum flott_storage_type
{
  FLOTT_DEV_MEM          = 1,       ///< read from static memory
  FLOTT_DEV_DEALLOC_MEM  = 1 << 2,  ///< read/write from/to dynamic memory
  FLOTT_DEV_FILE_TO_MEM  = 1 << 3,  ///< read file into memory
  FLOTT_DEV_FILE         = 1 << 4,  ///< read/write from/to file
  FLOTT_DEV_STDOUT       = 1 << 5,  ///< standard out
  FLOTT_DEV_STOP_SYMBOL  = 1 << 6   ///< stop symbol (future use -- not implemented yet)
};

struct flott_token
{
  flott_uint uid;              ///< unique id field
  //flott_uint length;           ///< run-length of token in symbols
  flott_uint previous_match;   ///< offset to previous match
  flott_uint next_match;       ///< offset to next match
  flott_uint previous_token;   ///< offset to previous token (to left)
  flott_uint next_token;       ///< offset to next token (to right)
};

struct flott_match_list
{
  flott_uint level;            ///< the level in which the list was created
  flott_uint length;           ///< length (number of elements in match list)
  flott_uint next_aggregate;   ///< next entry in aggregate list
  //flott_uint reserved;         ///< field has no use at this point
  flott_uint first_match;      ///< first entry in match list
  flott_uint last_match;       ///< last entry in match list
};

struct flott_token_list
{
  flott_uint length;            ///< length of token list
  flott_uint first_token;       ///< offset to first token
  flott_uint second_last_token; ///< offset to second-last token
};

struct flott_sequence
{
  bool deallocate;
  size_t length;
  size_t *member;
};

struct flott_source
{
  size_t length;                    ///< length of source in bytes
  size_t start_offset;
  size_t end_offset;
  flott_storage_type storage_type;  ///< type of input (i.e HD, memory)
  char *path;
  union { FILE *handle; char *bytes; } data;
  void* user;
};

struct flott_input
{
  bool deallocate;      ///< free dynamic memory upon a call to 'flott_destroy()'
  bool append_termchar; ///< append terminal character to end of input
  flott_symbol_type
    symbol_type;        ///< symbol width in bits
  size_t count;         ///< number of input sources
  size_t length;
  flott_source *source; ///< array holding input descriptors
  flott_sequence
    sequence;           ///< ordered sequence in which input is to be processed
};

struct flott_result
{
  flott_uint levels;
  double t_complexity;
  double t_information;
  double t_entropy;
};

struct flott_handler
{
  flott_destroy_handler *destroy;
  flott_init_handler *init;
  flott_progress_handler *progress;
  flott_step_handler *step;
  flott_message_handler *message;
  flott_error_handler *error;
};

struct flott_status
{
  int code;
  char message[FLOTT_LINE_BUFSZ];
};

struct flott_private
{
  void *base_pointer;   ///< base pointer to used memory block (set by init routine)
  double ln2;           ///< ln(2)

  size_t input_sequence_member;
  char output_buffer[FLOTT_PAGE_SIZE];

  flott_token_list
    token_list;         ///< header info for token list
  flott_uint
    allocation_length;  ///< total memory in token units
};

struct flott_object
{
  flott_input input;        ///< input descriptor
  flott_result result;      ///< t-transform result descriptor
  flott_status status;      ///< status codes/messages
  flott_handler handler;    ///< handler function pointers
  /* TODO: implement sliding window
   * flott_uint window_size;   ///< size of a sliding window (default = 0, no window) */
  flott_vlevel
    verbosity_level;        ///< verbosity level [0 - 5] (default = 0)
  flott_uint alphabet_size; ///< actual unique symbols found in input.
  void *user;               ///< pointer to custom user application data
  flott_private private;    ///< private object data (do not touch please)
};

flott_object *flott_create_instance (size_t input_source_count);
int flott_initialize (flott_object *op);
void flott_t_transform (flott_object *op);
void flott_inverse_t_transform (flott_object *op);
void flott_input_write (flott_object *op, size_t cp_start_offset,
                        size_t cp_length, FILE *output_handle);
void flott_deinitialize (flott_object *op);
void flott_destroy (flott_object *op);
int flott_set_status (flott_object *op, int code, const int level, ...);

/* provide normalized t-information distance prototypes */
#include "flott_nid.h"

#ifdef __cplusplus
}
#endif

#endif /* _FLOTT_H_ */
