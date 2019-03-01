/* 
 * High-level compiler to brainf**k
 * Copyright (C) 2019 theKidOfArcrania
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UTILS_LOC_H
#define _UTILS_LOC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  const char* sourceFile;
} YYLTYPE;

#define LCOPY_BEGIN(dst, src) do {          \
  const YYLTYPE* __src = &(src);            \
  (dst).first_line = __src->first_line;     \
  (dst).first_column = __src->first_column; \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)
#define LCOPY_END(dst, src) do {            \
  const YYLTYPE* __src = &(src);            \
  (dst).last_line = __src->last_line;       \
  (dst).last_column = __src->last_column;   \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)
#define LCOPY(dst, src) do {                \
  const YYLTYPE* __src = &(src);            \
  (dst).first_line = __src->first_line;     \
  (dst).first_column = __src->first_column; \
  (dst).last_line = __src->last_line;       \
  (dst).last_column = __src->last_column;   \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)

#ifdef __cplusplus
}
#endif

#endif // _ERRHAND_H
