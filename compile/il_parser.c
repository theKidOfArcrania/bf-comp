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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compile/ast.h"
#include "utils/list.h"
#include "utils/hashmap.h"
#include "utils/checkmem.h"
#include "utils/errhand.h"

enum frame_type {
  FRAME_VAR, FRAME_CTX
};

#define frame_ent(node) list_entry(node, struct frame_ele, frames)
struct frame_ele {
  uint32_t type;
  uint32_t ptr;
  char *iden;
  struct list_head frames;
};

static struct frame_ele *push_var(struct list_head *frames, const char *name, uint32_t *next_empty) {
  struct frame_ele *var = malloc_c(sizeof(*var));
  *var = (struct frame_ele) {.type = FRAME_VAR, .ptr = (*next_empty)++, 
    .iden = strdup_c(name)};
  list_add_tail(&var->frames, frames);
  return var;
}

static int pop_var(struct list_head *del, const char *match, uint32_t *next_empty) {
  if (frame_ent(del)->type != FRAME_VAR)
    return 0;
  if (match && strcmp(frame_ent(del)->iden, match))
    return 0;

  (*next_empty)--;

  list_del(del);
  free(frame_ent(del)->iden);
  free(frame_ent(del));
  return 1;
}

static void push_ctx(struct list_head *frames, uint32_t cur_ptr) {
  struct frame_ele *ctx = malloc_c(sizeof(*ctx));
  *ctx = (struct frame_ele) {.type = FRAME_CTX, .ptr = cur_ptr};
  list_add_tail(&ctx->frames, frames);
}

static void move_ptr(cstr *out, uint32_t *cur_ptr, uint32_t to) {
  while (*cur_ptr < to) {
    cstr_append_c(out, '>');
    (*cur_ptr)++;
  }

  while (*cur_ptr > to) {
    cstr_append_c(out, '<');
    (*cur_ptr)--;
  }
}

static cstr *optimize_repeat(const cstr *in) {

#define update(out, val, do_wrap, cneg, cpos) do { \
  char *__ch = ch_repeat((val), (do_wrap), (cneg), (cpos)); \
  cstr_append_s((out), __ch); \
  free(__ch); \
  (val) = 0; \
} while(0);

  cstr *out = cstr_new_i(in->length);

  size_t i;
  int shift = 0, was_add = 0, was_shift = 0, is_add, is_shift;
  char add = 0;
  for (i = 0; i < in->length; i++) {
    is_add = is_shift = 0;
    switch (in->str[i]) {
      case '-':
        is_add = 1;
        add--;
        break;
      case '+':
        is_add = 1;
        add++;
        break;
      case '<':
        is_shift = 1;
        shift--;
        break;
      case '>':
        is_shift = 1;
        shift++;
    }

    if (!is_add && was_add)
      update(out, add, 1, '-', '+');

    if (!is_shift && was_shift)
      update(out, shift, 0, '<', '>');

    was_shift = is_shift;
    was_add = is_add;

    if (!is_add && !is_shift)
      cstr_append_c(out, in->str[i]);
  }

  if (was_add)
    update(out, add, 1, '-', '+');

  if (was_shift)
    update(out, shift, 0, '<', '>');

  return out;
#undef update
}

cstr *optimize(const cstr *in) {
  return optimize_repeat(in);
}


cstr *compile_il(const struct list_head* ast) {
  map_t sym_table = hashmap_new();
  LIST_HEAD(frames);

  cstr *out = cstr_new_i(1000);
  struct list_head *node, *node2, *tmp;
  struct frame_ele *t_frame;
  stmt *s;

  uint32_t cur_ptr = 0, next_empty = 0, limit = UNLIMITED;

  list_for_each(node, ast) {
    s = stmt_ent(node);
    switch (s->type) {
      case STMT_LITERAL: 
        cstr_append_str(out, s->d.literal);
        break;
      case STMT_PUSHVAR:
        if (hashmap_get_def(sym_table, s->d.iden, NULL)) {
          lerror(&s->loc, "symbol `%s' already exists", s->d.iden);
          break;
        }

        if (next_empty >= limit)
          lerror(&s->loc, "Var definition passes limit of %d", limit);
        t_frame = push_var(&frames, s->d.iden, &next_empty);
        hashmap_put(sym_table, t_frame->iden, t_frame);
        break;
      case STMT_POPVAR:
        if (list_empty(&frames) || !pop_var(frames.prev, s->d.iden, &next_empty)) {
          lerror(&s->loc, "mismatched pop-var of symbol `%s'", s->d.iden);
          break;
        }

        hashmap_remove(sym_table, s->d.iden);
        break;
      case STMT_DELVAR:
        if (hashmap_remove(sym_table, s->d.iden) != MAP_OK)
          lerror(&s->loc, "symbol `%s' is invalid or has been deleted", s->d.iden);
        break;
      case STMT_ATVAR:
        if (hashmap_get(sym_table, s->d.iden, (void**)&t_frame) != MAP_OK) {
          lerror(&s->loc, "symbol `%s' is invalid or has been deleted", s->d.iden);
          t_frame = push_var(&frames, s->d.iden, &next_empty);
          hashmap_put(sym_table, t_frame->iden, t_frame);
          linfo(&s->loc, "each invalid symbol is reported once for each scope that it appears");
        }

        ASSERT(t_frame->type == FRAME_VAR);
        ASSERT(!strcmp(s->d.iden, t_frame->iden));

        move_ptr(out, &cur_ptr, t_frame->ptr);

        break;
      case STMT_PUSHCTX:
        push_ctx(&frames, cur_ptr);
        break;
      case STMT_POPCTX:
        list_for_each_prev(node2, &frames) {
          if (frame_ent(node2)->type == FRAME_CTX)
            goto do_popctx;
        }

        lerror(&s->loc, "mismatched pop-ctx");
        break;

do_popctx:
        list_for_each_prev_safe(node2, tmp, &frames) {
          if (frame_ent(node2)->type == FRAME_CTX) {
            move_ptr(out, &cur_ptr, frame_ent(node2)->ptr);
            list_del(node2);
            free(frame_ent(node2));
            break;
          }
          hashmap_remove(sym_table, frame_ent(node2)->iden);
          ASSERT(pop_var(node2, NULL, &next_empty));
        }
        break;
      case STMT_LIMIT:
        limit = s->d.ival;
        if (limit < next_empty)
          lerror(&s->loc, "New limit is already exceeded");
        break;
      case STMT_ARR_SHIFT:
        if (limit == UNLIMITED)
          lerror(&s->loc, "Must specify a limit to do array shifts!");
        else
          cstr_append_s(out, ch_repeat(s->d.sval * (int32_t)limit, 0, '<', '>'));
        break;
      default: 
        gfatal("switch error");
    }
  }

  list_for_each_prev_safe(node2, tmp, &frames) {
    if (frame_ent(node2)->type == FRAME_VAR) {
      ASSERT(pop_var(node2, NULL, &next_empty));
    } else {
      list_del(node2);
      free(frame_ent(node2));
    }
  }
  hashmap_free(sym_table);
  return out;
}



void dump_il(const struct list_head *ast, FILE *out) {
  struct list_head *node;
  stmt *s;

  fprintf(out, "### [BEGIN_BIL]\n");
  list_for_each(node, ast) {
    s = stmt_ent(node);
    switch (s->type) {
      case STMT_LITERAL: fprintf(out, "STMT_LITERAL(\"%s\")\n", s->d.literal->str); break;
      case STMT_PUSHVAR: fprintf(out, "STMT_PUSHVAR(%s)\n", s->d.iden); break;
      case STMT_POPVAR: fprintf(out, "STMT_POPVAR(%s)\n", s->d.iden); break;
      case STMT_DELVAR: fprintf(out, "STMT_DELVAR(%s)\n", s->d.iden); break;
      case STMT_ATVAR: fprintf(out, "STMT_ATVAR(%s)\n", s->d.iden); break;
      case STMT_PUSHCTX: fprintf(out, "STMT_PUSHCTX\n"); break;
      case STMT_POPCTX: fprintf(out, "STMT_POPCTX\n"); break;
      case STMT_LIMIT: fprintf(out, "STMT_LIMIT(%d)\n", s->d.ival); break;
      case STMT_ARR_SHIFT: fprintf(out, "STMT_ARR_SHIFT(%d)\n", s->d.sval); break;
      default: gfatal("switch error");
    }
  }
  fprintf(out, "### [END_BIL]\n");
}
