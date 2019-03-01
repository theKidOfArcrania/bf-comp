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

#include <stdlib.h>
#include <string.h>

#include "utils/checkmem.h"
#include "utils/cstr.h"

#define DEF_CSTR_CAP 0x10
#define grow(l) ((l) + ((l) >> 2))

cstr *cstr_new() {
  return cstr_new_i(DEF_CSTR_CAP);
}

cstr *cstr_new_i(size_t cap) {
  cstr *s = malloc_c(sizeof(*s));
  s->str = malloc_c(cap);
  s->cap = cap;
  s->length = 0;
  return s;
}

cstr *cstr_new_scp(const char *str) {
  cstr *s = malloc_c(sizeof(*s));
  
  size_t len = strlen(str);
  size_t cap = grow(len + 1);
  
  s->str = malloc_c(cap);
  s->str[len] = 0;
  memcpy(s->str, str, len);

  s->cap = cap;
  s->length = len;
  return s;
}

cstr *cstr_new_smv(char *str) {
  cstr *s = malloc_c(sizeof(*s));
  
  size_t len = strlen(str);
  s->str = str;
  s->length = len;
  s->cap = len + 1;
  return s;
}

void cstr_append_carr(cstr *s, const char *arr, size_t clen) {
  size_t newlen = s->length + clen;
  if (newlen >= s->cap) {
    size_t newcap = grow(s->cap);
    if (newlen >= newcap)
      newcap = newlen + 1;

    s->str = realloc_c(s->str, newcap);
    s->cap = newcap;
  }

  memcpy(s->str + s->length, arr, clen);
  s->str[newlen] = 0;
  s->length = newlen;
}

void cstr_append_s(cstr *s, const char *str) {
  cstr_append_carr(s, str, strlen(str));
}

void cstr_append_c(cstr *s, char c) {
  char buff[2] = {c, 0};
  cstr_append_carr(s, buff, 1);
}

void cstr_append_str(cstr *s, const cstr *t) {
  cstr_append_carr(s, t->str, t->length);
}

int cstr_insert_carr(cstr *s, size_t ind, const char *arr, size_t clen) {
  if (ind > s->length)
    return 0;


  size_t newlen = s->length + clen;
  if (newlen >= s->cap) {
    size_t newcap = grow(s->cap);
    if (newlen >= newcap)
      newcap = newlen + 1;

    s->str = realloc_c(s->str, newcap);
    s->cap = newcap;
  }

  if (ind < s->length)
    memmove(s->str + ind, s->str + ind + clen, s->length - ind);
  memcpy(s->str + ind, arr, clen);
  s->str[newlen] = 0;
  s->length = newlen;
  return 1;
}

int cstr_insert_s(cstr *s, size_t ind, const char *str) {
  return cstr_insert_carr(s, ind, str, strlen(str));
}

int cstr_insert_c(cstr *s, size_t ind, char c) {
  char buff[2] = {c, 0};
  return cstr_insert_carr(s, ind, buff, 1);
}

int cstr_insert_str(cstr *s, size_t ind, const cstr *t) {
  return cstr_insert_carr(s, ind, t->str, t->length);
}


void cstr_delete(cstr *s) {
  free(s->str);
  free(s);
}

