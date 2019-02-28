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

#include <string.h>
#include <stdlib.h>

#define _UTILS_CHECKMEM_C
#include "utils/checkmem.h"
#include "utils/errhand.h"

#define CHK(out) do {         \
  void* __val = (void*)(out); \
  if (!__val) {               \
    gfatal("Out of memory!"); \
  }                           \
  return __val;               \
} while(0)

void* malloc_c(size_t size) {
  CHK(malloc(size));
}
void* calloc_c(size_t nmemb, size_t size) {
  CHK(calloc(nmemb, size));
}
void* realloc_c(void *ptr, size_t size) {
  CHK(realloc(ptr, size));
}
char* strdup_c(const char *s) {
  CHK(strdup(s));
}
void* memdup_c(const void* src, size_t len) {
  void* dst = malloc_c(len);
  memcpy(dst, src, len);
  return dst;
}

