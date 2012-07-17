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
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "flott_platform.h"

#ifdef _MSC_VER
  #include <sys/types.h>
  /* stat is buggy in msvc 2008, use stat64 */
  #define stat _stat64
  #ifndef S_ISREG
    #define S_ISREG(mode) (((mode) & _S_IFMT) == _S_IFREG)
  #endif
#else
  #include <unistd.h>  
#endif

size_t
flott_get_file_size (char *filename)
{
  size_t ret_val = 0;
  struct stat info;
  /* get file info */
  if ( stat (filename, &info) == 0 && S_ISREG(info.st_mode))
    {
      ret_val = (size_t) (info.st_size);
    }

  return ret_val;
}

int
flott_file_exists (char *filename)
{
  int ret_val = 0;
  struct stat info;
  /* test if file found */
  if ( stat(filename, &info) == 0 && S_ISREG(info.st_mode))
  {
          ret_val = 1;
  }

  return ret_val;
}

int
flott_file_exists_not_empty (char *filename)
{
  int ret_val = 0;
  struct stat info;
  /* get file info */
  if ( stat (filename, &info) == 0 && S_ISREG(info.st_mode))
    {
      ret_val = (info.st_size > 0);
    }

  return ret_val;
}

size_t flott_load_file_to_memory(const char *filename, char **result)
{
  size_t size = 0;
  FILE *fp = fopen (filename, "rb");
  if (fp == NULL)
  {
      *result = NULL;
      return -1; /* -1 means file opening failed */
  }

  fseek (fp, 0, SEEK_END);
  size = ftell(fp);

  fseek (fp, 0, SEEK_SET);
  *result = (char *) malloc (size + 1);
  if (result != NULL)
    {

      if (size != fread (*result, sizeof (char), size, fp))
      {
          free (*result);
          *result = NULL;
          return -2; /* -2 means file reading failed */
      }
    }
  else
    {
      fclose (fp);
      return -3; /* -3 means malloc failed */
    }
  fclose (fp);

  return size;
}

int
flott_get_digit_count (uint64_t uint_val)
{
  int ret_val = 0;
  char uint_string[32]; /* this is enough for 64-bits ints */
  if ((ret_val = sprintf (uint_string, "%llu", uint_val)) < 0)
    {
      ret_val = 0;
    }

  return ret_val;
}
