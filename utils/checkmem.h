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

#ifndef _UTILS_CHECKMEM_H
#define _UTILS_CHECKMEM_H

#ifdef __cplusplus
extern "C" 
#endif

#include <sys/types.h>

#ifndef _UTILS_CHECKMEM_C

#define BUILD_ERROR $%^%$&*
#define malloc BUILD_ERROR
#define calloc BUILD_ERROR
#define realloc BUILD_ERROR
#define strdup BUILD_ERROR
#define memdup BUILD_ERROR

#endif



void* malloc_c(size_t size);
void* calloc_c(size_t nmemb, size_t size);
void* realloc_c(void *ptr, size_t size);
char* strdup_c(const char *s);
void* memdup_c(const void *mem, size_t len);

#ifdef __cplusplus
}
#endif

#endif 
