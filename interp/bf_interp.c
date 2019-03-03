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

#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include "utils/cstr.h"
#include "utils/checkmem.h"

enum bf_commands {
  T_END = 0, T_INVLOOP = 1, T_PLUS, T_MINUS, T_LEFT, T_RIGHT, T_LOOP,
  T_ENDLOOP, T_INPUT, T_OUTPUT
};

static cstr *read_bf(const char *file);
static void exec_bf(const cstr *scode);

int main(int argc, char **argv) {


  if (argc != 2) {
    printf("Usage: %s FILE\n", program_invocation_short_name);
    return 1;
  }

  cstr *code = read_bf(argv[1]);
  exec_bf(code);
  cstr_delete(code);
}



static cstr *read_bf(const char *file) {
  FILE *f;
  char buff[0x1000];
  cstr *out = cstr_new();
  int readn, ind;

  if (!(f = fopen(file, "r")))
    err(1, "%s", file);


  while (1) {
    if (!(readn = fread(buff, 1, sizeof(buff), f))) {
      if (feof(f))
        break;
      errno = ferror(f);
      err(1, "read failed");
    }

    for (ind = 0; ind < readn; ind++) {
      switch (buff[ind]) {
        case '+': cstr_append_c(out, T_PLUS); break;
        case '-': cstr_append_c(out, T_MINUS); break;
        case '<': cstr_append_c(out, T_LEFT); break;
        case '>': cstr_append_c(out, T_RIGHT); break;
        case '[': cstr_append_c(out, T_LOOP); break;
        case ']': cstr_append_c(out, T_ENDLOOP); break;
        case ',': cstr_append_c(out, T_INPUT); break;
        case '.': cstr_append_c(out, T_OUTPUT); break;
      }
    }
  }
  cstr_append_c(out, T_END);
  fclose(f);
  return out;
}

#define TAPE_SIZE 0x1000
#define TAPE_MASK (TAPE_SIZE - 1)
static void exec_bf(const cstr *scode) {

  uint32_t max_stk = 1;
  unsigned char *tape_base, *tp;
  intptr_t tp_top, tp_bot = 0;

  const char *pc = scode->str, **sp, **stack_base;
  char invc = T_INVLOOP;
  
  setbuf(stdout, NULL);

  while (*pc) {
    if (*pc == T_LOOP)
      max_stk++;
    pc++;
  }

  if ((errno = posix_memalign((void**)&tape_base, TAPE_SIZE, TAPE_SIZE)))
    err(1, "memalign() failed");
  bzero(tape_base, TAPE_SIZE);
  tp = tape_base;
  tp_top = (intptr_t)tp & ~TAPE_MASK;

  sp = stack_base = malloc_c(max_stk * sizeof(*sp));
  sp[0] = &invc; // If this gets popped, control exits as error
  pc = scode->str;
  while (1) {
    switch (*pc) {
      case T_END: goto done;
      case T_INVLOOP: errx(1, "mismatched [ and ]"); break;
      case T_PLUS: (*tp)++; break;
      case T_MINUS: (*tp)--; break;
      case T_LEFT: tp = (void*)(tp_top | (--tp_bot & TAPE_MASK)); break;
      case T_RIGHT: tp = (void*)(tp_top | (++tp_bot & TAPE_MASK)); break;
      case T_LOOP: 
        if (*tp)
          *(++sp) = pc; 
        else {
          max_stk = 1;
          while (max_stk) {
            pc++;
            if (*pc == T_ENDLOOP)
              max_stk--;
            else if (*pc == T_LOOP)
              max_stk++;
          }
        }
        break;
      case T_ENDLOOP: pc = *(sp--); goto no_pc;
      case T_INPUT: (*tp) = getchar(); break;
      case T_OUTPUT: putchar(*tp); break;
    }
    pc++;
no_pc: ;
  }
done:

  free(stack_base);
  free(tape_base);

#undef cur 
}

