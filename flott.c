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
#include <string.h>

#include "flott.h"
#include "flott_math.h"
#include "flott_util.h"
#include "flott_term.h"

/**
 * local function pointer type definitions
 */
typedef size_t (flott_init_symbols) (char *, size_t, flott_token *,
                                     size_t, flott_match_list *, size_t);

/**
 * implementation
 */
flott_object *
flott_create_instance (size_t input_source_count)
{
  flott_source *source = NULL;
  flott_object *op = (flott_object *) calloc (1, sizeof (flott_object));

  if (op != NULL)
    {
      op->input.symbol_type = FLOTT_SYMBOL_BYTE;

      if (input_source_count > 0)
        {
          source = (flott_source *) malloc (sizeof (flott_source)
                                            * input_source_count);
        }
      else
        {
          input_source_count = 0;
        }

      if (input_source_count > 0 && source == NULL)
        {
          /* could not allocate source array, cleaning up */
          free (op);
          op = NULL;
        }
      else
        {
          op->input.count = input_source_count;
          op->input.source = source;
        }
    }
  return op;
}

size_t
flott_initialize_bits (char *data,
                       size_t data_length,
                       flott_token *tl_bp,
                       size_t token_offset,
                       flott_match_list *ml_header_bp,
                       size_t ml_header_offset)
{
  /* bit mask lut used for fast bit isolation */
  static const size_t bit_mask[8] =
      { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

  int i;
  size_t data_byte;
  size_t data_ordinal;
  flott_match_list *ml_header;
  flott_token *ml_token;
  flott_token *tl_token = tl_bp + token_offset;

  while (data_length-- > 0)
    {
      data_byte = *data++;
      i = 8;
      while (--i >= 0)
        {
          /* printf("%x,%d  \n", bit_mask[i],  ((data_byte & bit_mask[i]) >> i) ); */
          data_ordinal = (data_byte & bit_mask[i]) >> i;

          /* printf("%d, %d\n", (int) token_offset, (int) data_ordinal);
          fflush(stdout); */

          ml_header = flott_get_ptr_M (ml_header_bp, data_ordinal);
          if (ml_header->length == 0)
            {
              ml_header->first_match =  (flott_uint) token_offset;
            }
          else
            {
              ml_token = flott_get_ptr_M (tl_bp, ml_header->last_match);
              ml_token->next_match = (flott_uint) token_offset;
            }

          tl_token->previous_match = ml_header->last_match;
          tl_token->next_match = FLOTT_NIL;

          ml_header->last_match = (flott_uint) token_offset;
          (ml_header->length)++;

          tl_token->uid = (flott_uint) (ml_header_offset + data_ordinal);
          tl_token->previous_token = (flott_uint) (token_offset - 1);
          tl_token->next_token = (flott_uint) ++token_offset;

          tl_token++;
        }
    }
  return token_offset;
}

size_t
flott_initialize_bytes (char *data,
                        size_t data_length,
                        flott_token *tl_bp,
                        size_t token_offset,
                        flott_match_list *ml_header_bp,
                        size_t ml_header_offset)
{
  size_t data_ordinal;
  flott_match_list *ml_header;
  flott_token *ml_token;
  flott_token *tl_token = tl_bp + token_offset;

  while (data_length-- > 0)
    {
      data_ordinal = (unsigned char) *data++;
      ml_header = flott_get_ptr_M (ml_header_bp, data_ordinal);
      if (ml_header->length == 0)
        {
          ml_header->first_match =  (flott_uint) token_offset;
        }
      else
        {
          ml_token = flott_get_ptr_M (tl_bp, ml_header->last_match);
          ml_token->next_match = (flott_uint) token_offset;
        }

      tl_token->previous_match = ml_header->last_match;
      tl_token->next_match = FLOTT_NIL;

      ml_header->last_match = (flott_uint) token_offset;
      (ml_header->length)++;

      tl_token->uid = (flott_uint) (ml_header_offset + data_ordinal);
      tl_token->previous_token = (flott_uint) (token_offset - 1);
      tl_token->next_token = (flott_uint) ++token_offset;

      tl_token++;
    }
  return token_offset;
}

size_t
flott_initialize_stop_symbol (char *data,
                        flott_token *tl_bp,
                        size_t token_offset,
                        flott_match_list *ml_header_bp,
                        size_t ml_header_offset)
{
  flott_match_list *ml_header;
  flott_token *ml_token;
  flott_token *tl_token = tl_bp + token_offset;

  ml_header = flott_get_ptr_M (ml_header_bp, FLOTT_STOP_SYMBOL);
  if (ml_header->length == 0)
    {
      ml_header->first_match =  (flott_uint) token_offset;
    }
  else
    {
      ml_token = flott_get_ptr_M (tl_bp, ml_header->last_match);
      ml_token->next_match = (flott_uint) token_offset;
    }

  tl_token->previous_match = ml_header->last_match;
  tl_token->next_match = FLOTT_NIL;

  ml_header->last_match = (flott_uint) token_offset;
  (ml_header->length)++;

  tl_token->uid = (flott_uint) (ml_header_offset + FLOTT_STOP_SYMBOL);
  tl_token->previous_token = (flott_uint) (token_offset - 1);
  tl_token->next_token = (flott_uint) ++token_offset;
  return token_offset;
}

size_t
flott_initialize_bytes_dna (char *data,
                            size_t data_length,
                            flott_token *tl_bp,
                            size_t token_offset,
                            flott_match_list *ml_header_bp,
                            size_t ml_header_offset)
{
  /* TODO: stub, implement use byte-wise for now */
  return token_offset;
}

int
flott_initialize_input (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;

  void *bp = op->private.base_pointer;
  flott_input input = op->input;
  flott_uint tl_length = op->private.token_list.length;

  size_t token_offset = 1;
  flott_token *tl_bp = (flott_token *) bp;

  size_t ml_header_offset = tl_length + 2;
  flott_match_list *ml_header_bp = ((flott_match_list *) bp) + ml_header_offset;
  size_t i, index, data_length;
  flott_match_list *ml_header = ml_header_bp;

  char *data, *filename;
  char data_page[16 * FLOTT_PAGE_SIZE]; ///< read 16 pages at a time
  FILE *fp;
  size_t read_bytes, total_read_bytes;

  flott_token *tl_token;
  flott_init_symbols *initialize_symbols;

  int offset_shift = 0;

  /* set function pointer to 1-bit/8-bit population routine */
  if (input.symbol_type == FLOTT_SYMBOL_BIT)
    {
      offset_shift = 3; ///< multiply offsets by 8
      initialize_symbols = &flott_initialize_bits;
    }
  else
    {
      initialize_symbols = &flott_initialize_bytes;
    }

  /* initialize level zero match list headers
   * (note: '<=' is no mistake; it's initializing the 'stop symbol' match list.) */
  for(i = 0; i <= FLOTT_SYMBOL_BYTE; i++)
    {
      memset (ml_header++, 0, sizeof (flott_match_list));
    }

  for(i = 0; i < op->input.sequence.length; i++)
    {
      index = op->input.sequence.member[i];
      data_length = input.source[index].length;
      input.source[index].start_offset = (token_offset - 1) << offset_shift;

      switch (input.source[index].storage_type)
        {
          case FLOTT_DEV_STOP_SYMBOL :
            token_offset = flott_initialize_stop_symbol (data,
                                                         tl_bp,
                                                         token_offset,
                                                         ml_header_bp,
                                                         ml_header_offset);
            break;
          case FLOTT_DEV_MEM :
          case FLOTT_DEV_DEALLOC_MEM :
            {
              /* populate flott data structures from memory pointer */
              data = input.source[index].data.bytes;
              if (data != NULL)
                {
                  token_offset = initialize_symbols (data,
                                                     data_length,
                                                     tl_bp,
                                                     token_offset,
                                                     ml_header_bp,
                                                     ml_header_offset);
                }
              else
                {
                  ret_val = flott_set_status (op, FLOTT_ERR_NULL_POINTER,
                                              FLOTT_VL_FATAL);
                }
            }
            break;
          case FLOTT_DEV_FILE_TO_MEM :
            {
              /* load entire file to memory and populate flott data structures */
              filename = input.source[index].path;
              if (flott_load_file_to_memory(filename, &data) == data_length)
                {
                  input.source[index].data.bytes = data;
                  token_offset = initialize_symbols (data,
                                                     data_length,
                                                     tl_bp,
                                                     token_offset,
                                                     ml_header_bp,
                                                     ml_header_offset);
                }
              else
                {
                  input.source[index].data.bytes = NULL;
                  ret_val = flott_set_status (op, FLOTT_ERR_LOADING_FILE,
                                              FLOTT_VL_FATAL, filename);
                }

            }
            break;
          case FLOTT_DEV_FILE :
            {
              /* TODO: move this in its own function */
              data = &data_page[0];
              input.source[index].data.bytes = NULL;
              filename = input.source[index].path;
              read_bytes = 0;
              total_read_bytes = 0;
              fp = fopen (filename, "rb");
              if (fp != NULL)
                {
                  while (true)
                    {
                      read_bytes = fread (data, 1, (16 * FLOTT_PAGE_SIZE), fp);
                      total_read_bytes += read_bytes;
                      token_offset = initialize_symbols (data,
                                                         read_bytes,
                                                         tl_bp,
                                                         token_offset,
                                                         ml_header_bp,
                                                         ml_header_offset);
                      /* check for end of file */
                      if (read_bytes != (16 * FLOTT_PAGE_SIZE))
                        {
                          fclose (fp);
                          if (total_read_bytes == data_length)
                            {
                              break;
                            }
                          else
                            {
                              ret_val = flott_set_status (op, FLOTT_ERR_LOADING_FILE,
                                                          FLOTT_VL_FATAL, filename);
                            }
                        }
                    }
                }
              else
                {
                  ret_val = flott_set_status (op, FLOTT_ERR_LOADING_FILE,
                                              FLOTT_VL_FATAL, filename);
                }
            }
            break;

          /* we should never get here */
          default:
            ret_val = flott_set_status (op, FLOTT_ERROR, FLOTT_VL_FATAL); break;
        }
      input.source[index].end_offset = (token_offset - 1) << offset_shift;
    }

  /* set seek position for 'flott_input_write' */
  op->private.input_sequence_member = i - 1;


  /* remove last symbol from input if no appended terminal character is used */
  if (op->input.append_termchar == false && op->private.token_list.length > 0)
    {
      tl_token = ((flott_token*) bp) + token_offset - 1;
      ml_header = ((flott_match_list*) bp) + tl_token->uid;
      (ml_header->length)--;
      ml_header->last_match = tl_token->previous_match;
      op->private.token_list.second_last_token = (flott_uint) token_offset - 2;
      (op->private.token_list.length)--;
    }
  else
    {
      op->private.token_list.second_last_token = (flott_uint) token_offset - 1;
    }

  /* update alphabet size with the actual number of unique symbols in input */
  if(ret_val == FLOTT_SUCCESS)
    {
      ml_header = ml_header_bp;
      for(i = 0; i < input.symbol_type; i++)
        {
          if (ml_header->length == 0) (op->alphabet_size)--;
          ml_header++;
        }
    }

  return ret_val;
}

int
flott_initialize (flott_object *op)
{
  int ret_val = FLOTT_SUCCESS;
  flott_uint tl_length;         ///< length of token list
  flott_token *tl_token;        ///< pointer to individual token in token list
  flott_uint max_input_length;  ///< maximum number of bytes allowed over parsing alphabet
  flott_uint allocation_length; ///< actual token list memory units allocated
  size_t input_length, i, index;
  void *bp; ///< base pointer to data structure memory

  if (op != NULL && op->input.source != NULL)
    {
      bp = op->private.base_pointer;

      /* handy scaling factor if t-information/t-entropy are measured in nats */
       op->private.ln2 = log (2.0);

      /* sum up the total length of all input sources in bytes */
      input_length = 0;
      if (op->input.sequence.length > 0)
        {
        for(i = 0; i < op->input.sequence.length; i++)
          {
            index = op->input.sequence.member[i];
            if (index < op->input.count)
              {
                input_length += op->input.source[index].length;
              }
            else
              {
                /* sequence index is out of bounds */
                ret_val = flott_set_status (op, FLOTT_ERR_INDEX_BOUNDS,
                                            FLOTT_VL_FATAL, " (sequence index)");
              }
          }
        }
      else /* use all sources, concatenated left-to-right */
        {
          op->input.sequence.deallocate = true; ///< auto deallocation
          op->input.sequence.member =
              (size_t *) malloc (op->input.count * sizeof (size_t));
          if (op->input.sequence.member != NULL)
            {
              op->input.sequence.length = op->input.count;
              for(i = 0; i < op->input.count; i++)
                {
                  op->input.sequence.member[i] = i;
                  input_length += op->input.source[i].length;
                }
            }
          else
            {
              ret_val = flott_set_status (op, FLOTT_ERR_MALLOC_FLOTT,
                                          FLOTT_VL_FATAL, " (sequence list)");
            }

        }

      max_input_length = FLOTT_UINT_MAX
                         - FLOTT_SYMBOL_BYTE
                         - 1  ///< stop symbol space
                         - 2; ///< space for head/tail node of token list

      /* check if we are supposed to use a binary source alphabet */
      if (op->input.symbol_type == FLOTT_SYMBOL_BIT)
        {
          input_length <<= 3; ///< multiply by 8
        }

      op->alphabet_size = op->input.symbol_type;

      /* make sure we don't exceed maximum offset/file size limit */
      if (input_length > max_input_length)
        {
          /* file is too large, we can't do a t-decomposition */
          ret_val = flott_set_status (op, FLOTT_ERR_MAX_INPUT_SIZE,
                                      FLOTT_VL_FATAL);
        }
      else
        {
          tl_length = (flott_uint) input_length;

          /* allocate memory for t-decomposition data structures */
          allocation_length = tl_length + FLOTT_SYMBOL_BYTE + 3;

          if (bp != NULL && (op->private.allocation_length < allocation_length))
            {
              free (bp);
              bp = NULL;
            }

          if (bp == NULL)
            {
              op->private.allocation_length = 0;
              bp = malloc (allocation_length * sizeof (flott_token));
              op->private.base_pointer = bp;
            }

          if (bp != NULL)
            {
              if (op->private.allocation_length == 0)
                {
                  op->private.allocation_length = allocation_length;
                }

              /* initialize token list head and tail nodes */
              tl_token = (flott_token *) bp;
              memset (tl_token, 0, sizeof (flott_token));
              tl_token->next_token = 1;

              tl_token = ((flott_token *) bp) + tl_length + 1;
              memset (tl_token, 0, sizeof (flott_token));
              tl_token->previous_token = tl_length;

              op->private.token_list.first_token = 1;
              op->private.token_list.length = tl_length;
              op->input.length = tl_length;

              /* load input(s) and initialize t-transform data structures */
              if ((ret_val = flott_initialize_input (op)) == FLOTT_SUCCESS)
                {
                  /* if set, call user initialization callback function */
                  if (op->handler.init != NULL)
                    {
                      ret_val = op->handler.init (op);
                    }
                }
            }
          else
            {
              ret_val = flott_set_status (op, FLOTT_ERR_MALLOC_FLOTT,
                                          FLOTT_VL_FATAL, " (token list)");
            }
        }
    }
  else
    {
      ret_val = flott_set_status (op, FLOTT_ERR_INVALID_OBJ,
                                  FLOTT_VL_FATAL, " (flott object)");
    }

  return ret_val;
}

/*TODO: needs to return an error no. */
void
flott_input_write (flott_object *op, size_t start_offset,
                   size_t count, FILE *output_handle)
{
  flott_private *private = &(op->private);
  size_t *seq_member = &(private->input_sequence_member);
  FILE *source_handle;
  flott_source *input_source;

  size_t data_offset, data_length, index, tmp_seq_member, seq_length;
  char *data;

  /* determine the input source in which the copy patterns starts */
  seq_length = *seq_member;
  while (*seq_member >= 0)
    {
      index = op->input.sequence.member[*seq_member];
      input_source = &(op->input.source[index]);

      if (input_source->start_offset <= start_offset)
        {
          break;
        }
      else
        {
          if (*seq_member != 0) (*seq_member)--;
        }
    }
  tmp_seq_member = *seq_member;
  seq_length = seq_length - *seq_member;

  /* write copy pattern to output device */
  while (count > 0)
    {
      data_offset = start_offset - input_source->start_offset;
      data_length = (input_source->end_offset - start_offset);

      /* check if copy pattern is spanning multiple input sources */
      if (count > data_length)
        {
          count -= data_length;
        }
      else
        {
          data_length = count;
          count = 0;
        }

      if ( input_source->storage_type
           & (FLOTT_DEV_MEM | FLOTT_DEV_DEALLOC_MEM
              | FLOTT_DEV_FILE_TO_MEM | FLOTT_DEV_STOP_SYMBOL) )
        {
          data = &(input_source->data.bytes[data_offset]);
          fwrite (data, sizeof (char), data_length, output_handle);
        }
      else if (input_source->storage_type == FLOTT_DEV_FILE)
        {
          source_handle = input_source->data.handle;
          if (source_handle == NULL)
            {
              /*TODO check for error*/
              source_handle = fopen (input_source->path, "rb");
              input_source->data.handle = source_handle;
            }
          data = private->output_buffer;
          /* set file offset from which to read */
          /*TODO check if handle NULL, error out */
          fseek (source_handle, data_offset, SEEK_SET);
          while (true)
            {
              if (data_length > FLOTT_PAGE_SIZE)
                {
                  /* TODO: error check */
                  fread (data, sizeof (char), FLOTT_PAGE_SIZE, source_handle);
                  fwrite (data, sizeof (char), FLOTT_PAGE_SIZE, output_handle);
                  data_length -= FLOTT_PAGE_SIZE;
                }
              else
                {
                  /* TODO: error check */
                  fread (data, sizeof (char), data_length, source_handle);
                  fwrite (data, sizeof (char), data_length, output_handle);
                  break;
                }
            }
        }
      else
        {
          /* TODO error out */
        }

      /* if needed, continue with next input source */
      if (count != 0)
        {
          index = op->input.sequence.member[++tmp_seq_member];
          input_source = &(op->input.source[index]);
          start_offset = input_source->start_offset;
        }
    }

  /* close file handles of open, no longer needed files */
  while (seq_length > 0)
    {
      index = op->input.sequence.member[*seq_member + seq_length--];
      input_source = &(op->input.source[index]);
      if (input_source->storage_type == FLOTT_DEV_FILE
          && input_source->data.handle != NULL)
        {
          fclose (input_source->data.handle);
          input_source->data.handle = NULL;
        }

    }
}

void
flott_t_transform_callback (flott_object *op)
{
  void *bp = op->private.base_pointer;
  double t_complexity = 0.0; ///< holds sum resulting in t-complexity
  int terminate = false;

  flott_token_list *tl_header = &(op->private.token_list);
  flott_uint tl_length = tl_header->length;

  flott_token *tl_bp = (flott_token *) bp;
  flott_match_list *ml_header_bp = (flott_match_list *) bp;

  flott_match_list *cp_ml_header, *aggregate_ml_header;

  flott_token *cp_token, *first_cp_match, *last_cp_match,
              *aggregate_token, *ml_token, *al_token;

  size_t cp_uid, cp_length, sl_token_offset,
         aggregate_token_offset, aggregate_token_length,
         al_header_offset, al_token_offset,
         joined_cp, joined_cp_length;

  flott_uint cf_value;
  ptrdiff_t ml_slot_offset;

  flott_uint tl_progress_length = tl_length;
  flott_uint tl_progress_dec = tl_length >> 6; ///< divide by 64

  flott_uint level = 0; ///< t-augmentation level

  /* get pointer to copy pattern token of t-augmentation level zero */
  sl_token_offset = tl_header->second_last_token;
  cp_token = flott_get_ptr_M (tl_bp, sl_token_offset);

  while (tl_length > 0)
    {
      /* call t-transform progress handler function */
      if (tl_progress_length >= tl_length)
        {
          if (tl_progress_length < tl_progress_dec)
            {
              tl_progress_length = 0;
            }
          else
            {
              tl_progress_length -= tl_progress_dec;
            }
          if (op->handler.progress != NULL)
            {
              op->handler.progress (op,
                  (1 - (float) tl_length / tl_header->length));
            }
        }

      /* increment t-augmentation level */
      level++;

      /* get length and unique id (uid) of copy pattern token */
      cp_length = sl_token_offset - cp_token->previous_token;
      cp_uid = cp_token->uid;

      /* move pointer to token to the left of the copy pattern */
      cp_token = flott_get_ptr_M (tl_bp, cp_token->previous_token);

      /* get match list header assigned to copy pattern and match list length */
      cp_ml_header = flott_get_ptr_M (ml_header_bp, cp_uid);

      /* determine copy factor and remove the run of copy pattern tokens
       * from token and match list.
       * (right-to-left parsing step) */
      cf_value = 1;
      joined_cp_length = cp_length;
      while (cp_token->uid == (flott_uint) cp_uid)
        {
          cf_value++;
          joined_cp_length += cp_length;
          cp_token -= cp_length;
        }

      /* update t-complexity value for t-augmentation step */
      t_complexity += flott_log2_M (cf_value + 1);

      /* call t-transform step handler function*/
      if (op->handler.step != NULL)
        {
          op->handler.step (op, level, cf_value,
                            (sl_token_offset - cp_length),
                            cp_length, joined_cp_length,
                            t_complexity,  &terminate);
          if (terminate == true) break; ///< terminate early
        }

      sl_token_offset -= joined_cp_length;

      /* update list lengths, since the above copy pattern tokens are located
       * at the end of their match and the end of the token list it is
       * sufficient to just update the list length, i.e. there is no need to
       * update 'previous' and 'next' offsets */
      tl_length -= cf_value;
      cp_ml_header->length -= cf_value;

      if (cp_ml_header->length == 0)
        {
          continue;
        }

      /* scan from left-to-right, chain up the maximum number of copy
       * patterns, and merge the run of copy patterns with the immediately
       * following token into a new aggregate token. */
      while (cp_ml_header->length > 0)
        {
          first_cp_match = flott_get_ptr_M (tl_bp, cp_ml_header->first_match);
          last_cp_match = first_cp_match;

          /* join the maximum number of copy pattern tokens */
          joined_cp = 1;
          joined_cp_length = cp_length;

          while (last_cp_match->next_token == last_cp_match->next_match
                 && joined_cp < cf_value)
            {
              joined_cp++;
              joined_cp_length += cp_length;

              /* initialize the augmentation level and length of possible
               * new aggregate match list headers */
              ((flott_match_list *) last_cp_match)->level = level;
              ((flott_match_list *) last_cp_match)->length = 0;

              last_cp_match += cp_length;
            }
          cp_ml_header->length -= (flott_uint) joined_cp;
          tl_length -= (flott_uint) joined_cp;

          /* remove joined copy patterns form match list */
          cp_ml_header->first_match = last_cp_match->next_match;

          /* determine aggregate token */
          aggregate_token_offset = last_cp_match->next_token;
          aggregate_token = flott_get_ptr_M (tl_bp, aggregate_token_offset);
          aggregate_token_length = aggregate_token_offset
                                       - aggregate_token->previous_token;

          /* update token list offsets to include the aggregate token */
          aggregate_token->previous_token = first_cp_match->previous_token;

          /* update previous token list token */
          (first_cp_match - cp_length)->next_token =
              (flott_uint) aggregate_token_offset;

          /* get match list header for aggregate token */
          al_header_offset = (size_t) (aggregate_token->uid);
          aggregate_ml_header = flott_get_ptr_M (ml_header_bp, al_header_offset);

          /* remove aggregate token from its former match list */
          (aggregate_ml_header->length)--;
          if ( (aggregate_ml_header->first_match == aggregate_token_offset)
               || al_header_offset == cp_uid )
            {
              /* removal from head if former match list */
              aggregate_ml_header->first_match = aggregate_token->next_match;
            }
          else
            {
              ml_token =
                  flott_get_ptr_M (tl_bp, aggregate_token->previous_match);
              ml_token->next_match = aggregate_token->next_match;

              ml_token =
                  flott_get_ptr_M (tl_bp, aggregate_token->next_match);
              ml_token->previous_match = aggregate_token->previous_match;
            }

          /* determine new match list and uid for aggregate token */
          while (true)
            {
              /* first time we have generated the aggregate token */
              if (aggregate_ml_header->level != level)
                {
                  /* set offset to next aggregate match list */
                  aggregate_ml_header->level = level;
                  aggregate_ml_header->next_aggregate =
                      (flott_uint) aggregate_token_offset;

                  /* initialize new match list and append aggregate token */
                  aggregate_ml_header = (flott_match_list *) last_cp_match;
                  aggregate_ml_header->level = 0;
                  aggregate_ml_header->length = 1;
                  aggregate_ml_header->first_match =
                      (flott_uint) aggregate_token_offset;
                  aggregate_ml_header->last_match =
                      (flott_uint) aggregate_token_offset;

                  /* update aggregate token info */
                  aggregate_token->previous_match = FLOTT_NIL;
                  aggregate_token->next_match = FLOTT_NIL;
                  aggregate_token->uid = (flott_uint) (aggregate_token_offset
                                             - aggregate_token_length);

                  break;
                }
              else /* we might have generated the aggregate token before */
                {
                  al_token_offset = (size_t) (aggregate_ml_header->next_aggregate);
                  al_token = flott_get_ptr_M (tl_bp, al_token_offset);

                  /* check if we have a match list already */
                  ml_slot_offset = (ptrdiff_t) (al_token->previous_token)
                                   + (ptrdiff_t) (joined_cp_length);

                  if ( ( (ptrdiff_t) (al_token->uid) - ml_slot_offset ) >= 0 )
                    {
                      aggregate_ml_header =
                          flott_get_ptr_M (ml_header_bp, ml_slot_offset);

                      /* append aggregate token to its match list */
                      if (aggregate_ml_header->length == 0)
                      {
                          aggregate_ml_header->first_match =
                              (flott_uint) aggregate_token_offset;

                          aggregate_token->previous_match = FLOTT_NIL;
                      }
                      else
                      {
                          ml_token =
                              flott_get_ptr_M (tl_bp,
                                               aggregate_ml_header->last_match);
                          ml_token->next_match =
                              (flott_uint) aggregate_token_offset;

                          aggregate_token->previous_match =
                              aggregate_ml_header->last_match;

                          aggregate_ml_header->last_match =
                              (flott_uint) aggregate_token_offset;
                      }
                      aggregate_ml_header->last_match =
                          (flott_uint) aggregate_token_offset;
                      (aggregate_ml_header->length)++;

                      /* update aggregate token info */
                      aggregate_token->next_match = FLOTT_NIL;
                      aggregate_token->uid = (flott_uint) (ml_slot_offset);

                      break;
                    }

                  /* no match list found, loop and check next aggregate offset */
                  aggregate_ml_header =
                      flott_get_ptr_M (ml_header_bp, al_token->uid);
                }
            }
        }
    }

  /* update progress bar to 100% done */
  if (op->handler.progress != NULL)
    {
      op->handler.progress (op, 1.0);
    }

  /* set results for levels, t-complexity, t-information, t-entropy */
  op->result.levels = level;
  op->result.t_complexity = t_complexity;
  op->result.t_information = flott_get_t_information (t_complexity);
  op->result.t_entropy = op->result.t_information / (tl_header->length + 1);
}

void
flott_t_transform_simple (flott_object *op)
{
  void *bp = op->private.base_pointer;
  double t_complexity = 0.0; ///< holds sum resulting in t-complexity

  flott_token_list *tl_header = &(op->private.token_list);
  flott_uint tl_length = tl_header->length;

  flott_token *tl_bp = (flott_token *) bp;
  flott_match_list *ml_header_bp = (flott_match_list *) bp;

  flott_match_list *cp_ml_header, *aggregate_ml_header;

  flott_token *cp_token, *first_cp_match, *last_cp_match,
              *aggregate_token, *ml_token, *al_token;

  size_t cp_uid, cp_length, sl_token_offset,
         aggregate_token_offset, aggregate_token_length,
         al_header_offset, al_token_offset,
         joined_cp, joined_cp_length;

  flott_uint cf_value;
  ptrdiff_t ml_slot_offset;

  flott_uint level = 0; ///< t-augmentation level

  /* get pointer to copy pattern token of t-augmentation level zero */
  sl_token_offset = tl_header->second_last_token;
  cp_token = flott_get_ptr_M (tl_bp, sl_token_offset);

  while (tl_length > 0)
    {
      /* increment t-augmentation level */
      level++;

      /* get length and unique id (uid) of copy pattern token */
      cp_length = sl_token_offset - cp_token->previous_token;
      cp_uid = cp_token->uid;

      /* move pointer to token to the left of the copy pattern */
      cp_token = flott_get_ptr_M (tl_bp, cp_token->previous_token);

      /* get match list header assigned to copy pattern and match list length */
      cp_ml_header = flott_get_ptr_M (ml_header_bp, cp_uid);

      /* determine copy factor and remove the run of copy pattern tokens
       * from token and match list.
       * (right-to-left parsing step) */
      cf_value = 1;
      joined_cp_length = cp_length;
      while (cp_token->uid == (flott_uint) cp_uid)
        {
          cf_value++;
          joined_cp_length += cp_length;
          cp_token -= cp_length;
        }

      /* update t-complexity value for t-augmentation step */
      t_complexity += flott_log2_M (cf_value + 1);

      sl_token_offset -= joined_cp_length;

      /* update list lengths, since the above copy pattern tokens are located
       * at the end of their match and the end of the token list it is
       * sufficient to just update the list length, i.e. there is no need to
       * update 'previous' and 'next' offsets */
      tl_length -= cf_value;
      cp_ml_header->length -= cf_value;

      if (cp_ml_header->length == 0)
        {
          continue;
        }

      /* scan from left-to-right, chain up the maximum number of copy
       * patterns, and merge the run of copy patterns with the immediately
       * following token into a new aggregate token. */
      while (cp_ml_header->length > 0)
        {
          first_cp_match = flott_get_ptr_M (tl_bp, cp_ml_header->first_match);
          last_cp_match = first_cp_match;

          /* join the maximum number of copy pattern tokens */
          joined_cp = 1;
          joined_cp_length = cp_length;

          while (last_cp_match->next_token == last_cp_match->next_match
                 && joined_cp < cf_value)
            {
              joined_cp++;
              joined_cp_length += cp_length;

              /* initialize the augmentation level and length of possible
               * new aggregate match list headers */
              ((flott_match_list *) last_cp_match)->level = level;
              ((flott_match_list *) last_cp_match)->length = 0;

              last_cp_match += cp_length;
            }
          cp_ml_header->length -= (flott_uint) joined_cp;
          tl_length -= (flott_uint) joined_cp;

          /* remove joined copy patterns form match list */
          cp_ml_header->first_match = last_cp_match->next_match;

          /* determine aggregate token */
          aggregate_token_offset = last_cp_match->next_token;
          aggregate_token = flott_get_ptr_M (tl_bp, aggregate_token_offset);
          aggregate_token_length = aggregate_token_offset
                                       - aggregate_token->previous_token;

          /* update token list offsets to include the aggregate token */
          aggregate_token->previous_token = first_cp_match->previous_token;

          /* update previous token list token */
          (first_cp_match - cp_length)->next_token =
              (flott_uint) aggregate_token_offset;

          /* get match list header for aggregate token */
          al_header_offset = (size_t) (aggregate_token->uid);
          aggregate_ml_header = flott_get_ptr_M (ml_header_bp, al_header_offset);

          /* remove aggregate token from its former match list */
          (aggregate_ml_header->length)--;
          if ( (aggregate_ml_header->first_match == aggregate_token_offset)
               || al_header_offset == cp_uid )
            {
              /* removal from head if former match list */
              aggregate_ml_header->first_match = aggregate_token->next_match;
            }
          else
            {
              ml_token =
                  flott_get_ptr_M (tl_bp, aggregate_token->previous_match);
              ml_token->next_match = aggregate_token->next_match;

              ml_token =
                  flott_get_ptr_M (tl_bp, aggregate_token->next_match);
              ml_token->previous_match = aggregate_token->previous_match;
            }

          /* determine new match list and uid for aggregate token */
          while (true)
            {
              /* first time we have generated the aggregate token */
              if (aggregate_ml_header->level != level)
                {
                  /* set offset to next aggregate match list */
                  aggregate_ml_header->level = level;
                  aggregate_ml_header->next_aggregate =
                      (flott_uint) aggregate_token_offset;

                  /* initialize new match list and append aggregate token */
                  aggregate_ml_header = (flott_match_list *) last_cp_match;
                  aggregate_ml_header->level = 0;
                  aggregate_ml_header->length = 1;
                  aggregate_ml_header->first_match =
                      (flott_uint) aggregate_token_offset;
                  aggregate_ml_header->last_match =
                      (flott_uint) aggregate_token_offset;

                  /* update aggregate token info */
                  aggregate_token->previous_match = FLOTT_NIL;
                  aggregate_token->next_match = FLOTT_NIL;
                  aggregate_token->uid = (flott_uint) (aggregate_token_offset
                                             - aggregate_token_length);

                  break;
                }
              else /* we might have generated the aggregate token before */
                {
                  al_token_offset = (size_t) (aggregate_ml_header->next_aggregate);
                  al_token = flott_get_ptr_M (tl_bp, al_token_offset);

                  /* check if we have a match list already */
                  ml_slot_offset = (ptrdiff_t) (al_token->previous_token)
                                   + (ptrdiff_t) (joined_cp_length);

                  if ( ( (ptrdiff_t) (al_token->uid) - ml_slot_offset ) >= 0 )
                    {
                      aggregate_ml_header =
                          flott_get_ptr_M (ml_header_bp, ml_slot_offset);

                      /* append aggregate token to its match list */
                      if (aggregate_ml_header->length == 0)
                      {
                          aggregate_ml_header->first_match =
                              (flott_uint) aggregate_token_offset;

                          aggregate_token->previous_match = FLOTT_NIL;
                      }
                      else
                      {
                          ml_token =
                              flott_get_ptr_M (tl_bp,
                                               aggregate_ml_header->last_match);
                          ml_token->next_match =
                              (flott_uint) aggregate_token_offset;

                          aggregate_token->previous_match =
                              aggregate_ml_header->last_match;

                          aggregate_ml_header->last_match =
                              (flott_uint) aggregate_token_offset;
                      }
                      aggregate_ml_header->last_match =
                          (flott_uint) aggregate_token_offset;
                      (aggregate_ml_header->length)++;

                      /* update aggregate token info */
                      aggregate_token->next_match = FLOTT_NIL;
                      aggregate_token->uid = (flott_uint) (ml_slot_offset);

                      break;
                    }

                  /* no match list found, loop and check next aggregate offset */
                  aggregate_ml_header =
                      flott_get_ptr_M (ml_header_bp, al_token->uid);
                }
            }
        }
    }

  /* set results for levels, t-complexity, t-information, t-entropy */
  op->result.levels = level;
  op->result.t_complexity = t_complexity;
  op->result.t_information = flott_get_t_information (t_complexity);
  op->result.t_entropy = op->result.t_information / (tl_header->length + 1);
}

void
flott_t_transform (flott_object *op)
{
  if (op->handler.progress != NULL || op->handler.step != NULL)
    {
      flott_t_transform_callback (op);
    }
  else
    {
      /* no use of callback functions (faster) */
      flott_t_transform_simple (op);
    }
}

void
flott_inverse_t_transform (flott_object *op)
{
  /* TODO: ditto. */
}

int
flott_set_status (flott_object *op, int code, const int vlevel, ...)
{
  va_list args;
  va_list propagate_args;
  int propagate = true;
  FILE* outdev = stdout;
  char* format;
  char* message_type = "";

  if (op != NULL)
    {
      if (vlevel == FLOTT_VL_FATAL
          || op->verbosity_level >= vlevel)
        {
          switch (vlevel)
            {
              case FLOTT_VL_FATAL : {
                                      message_type = "[fatal] ";
                                      outdev = stderr;
                                    }
                                    break;
              case FLOTT_VL_WARN  : message_type = "[warning] "; break;
              case FLOTT_VL_INFO  : message_type = "[info] "; break;
              case FLOTT_VL_DEBUG : message_type = "[debug] "; break;
              case FLOTT_VL_LOG   : message_type = "[log] "; break;
              default             : break;
            }

          /* get variable argument list */
          va_start(args, vlevel);

          /* determine and set message formatting */
          if (code == FLOTT_CUSTOM_ERROR)
            {
              code = va_arg (args, int);
              format = (char *) va_arg (args, char *);
            }
          else if (code == FLOTT_CUSTOM_MSG)
            {
              format = (char *) va_arg (args, char *);
            }
          else if (code <= FLOTT_ERROR)
            {
              format = (char *) (flott_errmsg_lut_G[~code + 1 /* flip sign */]);
            }
          else if (code > FLOTT_CUSTOM_MSG)
            {
              format = (char *) (flott_msg_lut_G[code]);
            }

          /*  if set, trigger error/message callback handler functions */
          if (op->verbosity_level == FLOTT_VL_FATAL)
            {
              if (op->handler.error != NULL)
                {
                  op->handler.error (op, &propagate);
                }
            }
          else
            {
              if (op->handler.message != NULL)
                {
                  op->handler.message (op, vlevel, &propagate);
                }
            }

          /* make a propagation copy of the arguments as some unix systems
           * won't let us reuse the argument list pointer */
          va_copy (propagate_args, args);

          /* write error message and error code to flott object */
          op->status.code = code;
          vsnprintf(op->status.message, FLOTT_LINE_BUFSZ, format, args);

          /* if 'propagate' is set write message/error to stdout/stderr */
          if (propagate == true)
            {
              if (vlevel == FLOTT_VL_FATAL || op->verbosity_level >= vlevel)
                {
                  fprintf (outdev, "%s", message_type);
                  vfprintf (outdev, format, propagate_args);
                  fprintf (outdev, "\n");
                }
            }

          /* clean up */
          va_end (args);
          va_end (propagate_args);
        }
    }

  return code;
}

void
flott_deinitialize (flott_object *op)
{
  size_t i = op->input.count;
  flott_source *source;
  if (op != NULL)
    {
      if (op->private.base_pointer != NULL)
      {
        free (op->private.base_pointer);
        op->private.base_pointer = NULL;
      }

      if (op->input.source != NULL)
      {
        while (i-- > 0)
          {
            source = &(op->input.source[i]);
            if (source->storage_type == FLOTT_DEV_FILE
                && source->data.handle != NULL)
              {
                fclose (source->data.handle);
                source->data.handle = NULL;
              }
          }
      }
    }
}

void
flott_destroy (flott_object *op)
{
  flott_source *source;

  if (op != NULL)
    {
      /* call destroy callback function if set */
      if (op->handler.destroy != NULL)
        {
          op->handler.destroy (op);
        }

      if (op->input.deallocate == true)
        {
          if (op->input.source != NULL)
            {
              while ((op->input.count)-- > 0)
                {
                  source = &(op->input.source[op->input.count]);
                  if ( source->storage_type &
                       (FLOTT_DEV_DEALLOC_MEM | FLOTT_DEV_FILE_TO_MEM) )
                    {
                      if (source->data.bytes != NULL)
                      {
                          free (source->data.bytes);
                      }
                    }
                  else if (source->storage_type == FLOTT_DEV_FILE
                           && source->data.handle != NULL)
                    {
                      fclose (source->data.handle);
                    }
                }
              free (op->input.source);
            }

          if (op->input.sequence.deallocate == true)
            {
              free (op->input.sequence.member);
            }
        }
      if (op->private.base_pointer != NULL) free (op->private.base_pointer);

      free (op);
      op = NULL;
    }
}
