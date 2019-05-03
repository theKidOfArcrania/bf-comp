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

#ifndef _COMPILE_ILPARSER_H
#define _COMPILE_ILPARSER_H

#include "utils/cstr.h"
#include "utils/listdef.h"

typedef struct _IO_FILE FILE;

cstr *compile_il(const struct list_head* ast);
void dump_il(const struct list_head *ast, FILE *out);
cstr *optimize(cstr *in);

#endif
