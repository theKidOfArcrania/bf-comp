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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compile/ast.h"
#include "utils/cstr.h"
#include "utils/list.h"
#include "utils/checkmem.h"
#include "utils/errhand.h"

#define STMT_VAL(t, l, v) ({ \
  stmt *__s = malloc_c(sizeof(stmt)); \
  __s->type = (t); \
  LCOPY(__s->loc, *(l)); \
  __s->d.ival = (v); \
  __s; \
  })
#define STMT_PTR(t, l, p) ({ \
  stmt *__s = malloc_c(sizeof(stmt)); \
  __s->type = (t); \
  LCOPY(__s->loc, *(l)); \
  __s->d.ptr = (p); \
  __s; \
  })
#define STMT_NONE(t, l) ({ \
  stmt *__s = malloc_c(sizeof(stmt)); \
  LCOPY(__s->loc, *(l)); \
  __s->type = (t); \
  __s; \
  })

char *ch_repeat(int val, int do_wrap, char cneg, char cpos) {
  int is_neg = 0;

  if (do_wrap) {
    int pos_val = val & 0xff;
    int neg_val = 256 - pos_val;
    if (pos_val < neg_val) {
      val = pos_val;
    } else {
      is_neg = 1;
      val = neg_val;
    }
  } else {
    if (val < 0) {
      is_neg = 1;
      val = -val;
    }
  }

  char *ret = malloc_c(val + 1);
  memset(ret, is_neg ? cneg : cpos, val);
  ret[val] = 0;
  return ret;
}

stmt *stmt_literal(const YYLTYPE* loc, cstr *lit) {
  return STMT_PTR(STMT_LITERAL, loc, lit);
}

stmt *stmt_pushvar(const YYLTYPE* loc, char *iden) {
  return STMT_PTR(STMT_PUSHVAR, loc, iden);
}

stmt *stmt_popvar(const YYLTYPE* loc, char *iden) {
  return STMT_PTR(STMT_POPVAR, loc, iden);
}

stmt *stmt_delvar(const YYLTYPE* loc, char *iden) {
  return STMT_PTR(STMT_DELVAR, loc, iden);
}

stmt *stmt_atvar(const YYLTYPE* loc, char *iden) {
  return STMT_PTR(STMT_ATVAR, loc, iden);
}

stmt *stmt_pushctx(const YYLTYPE* loc) {
  return STMT_NONE(STMT_PUSHCTX, loc);
}

stmt *stmt_popctx(const YYLTYPE* loc) {
  return STMT_NONE(STMT_POPCTX, loc);
}

stmt *stmt_limit(const YYLTYPE* loc, uint32_t lim) {
  return STMT_VAL(STMT_LIMIT, loc, lim);
}

stmt *stmt_arr_shift(const YYLTYPE* loc, int32_t shift) {
  return STMT_VAL(STMT_ARR_SHIFT, loc, (uint32_t)shift);
}


void stmt_delete(stmt *s) {
  switch (s->type) {
    case STMT_LITERAL: cstr_delete(s->d.literal); break;
    case STMT_PUSHVAR: free(s->d.ptr); break;
    case STMT_POPVAR: free(s->d.ptr); break;
    case STMT_ATVAR: free(s->d.ptr); break;
    case STMT_DELVAR: free(s->d.ptr); break;
    case STMT_PUSHCTX:
    case STMT_POPCTX: 
    case STMT_ARR_SHIFT:
    case STMT_LIMIT: break;
    default: gerror("switch error");
  }
  free(s);
}

void s_add_literal(struct list_head *stmts, const YYLTYPE* loc, char *lit) {
  if (stmt_ent(stmts->prev)->type == STMT_LITERAL) {
    cstr_append_s(stmt_ent(stmts->prev)->d.literal, lit);
    free(lit);
  } else {
    s_add(stmts, stmt_literal, loc, cstr_new_smv(lit));
  }
}


static char *nexttmp() {
  static int nextvar = 0;
  char *ret;
  if (asprintf(&ret, "$%d", nextvar) < 0)
    gfatal("Out of memory!");

  nextvar++;
  return ret;
}

char *stmts_tmpvar(struct list_head *stmts, const YYLTYPE* loc, int is_zero) {
  char *temp = nexttmp();
  s_add(stmts, stmt_pushvar, loc, strdup_c(temp));
  if (is_zero) {
    s_add(stmts, stmt_atvar, loc, strdup_c(temp));
    s_add(stmts, stmt_literal, loc, cstr_new_scp("[-]"));
  }
  return temp;
}

char *stmts_var_maketemp(struct list_head *stmts, const YYLTYPE* loc, char *iden) {
  char *temp1, *temp2;

  temp1 = stmts_tmpvar(stmts, loc, 1);
  temp2 = stmts_tmpvar(stmts, loc, 1);

  s_add(stmts, stmt_atvar, loc, iden);
  s_add(stmts, stmt_literal, loc, cstr_new_scp("[-"));
  s_add(stmts, stmt_atvar, loc, strdup_c(temp1));
  s_add(stmts, stmt_literal, loc, cstr_new_scp("+"));
  s_add(stmts, stmt_atvar, loc, temp2);
  s_add(stmts, stmt_literal, loc, cstr_new_scp("+"));
  s_add(stmts, stmt_atvar, loc, strdup_c(iden));
  s_add(stmts, stmt_literal, loc, cstr_new_scp("]"));

  s_add(stmts, stmt_atvar, loc, strdup_c(temp2));
  s_add(stmts, stmt_literal, loc, cstr_new_scp("[-"));
  s_add(stmts, stmt_atvar, loc, strdup_c(iden));
  s_add(stmts, stmt_literal, loc, cstr_new_scp("+"));
  s_add(stmts, stmt_atvar, loc, strdup_c(temp2));
  s_add(stmts, stmt_literal, loc, cstr_new_scp("]"));

  s_add(stmts, stmt_popvar, loc, strdup_c(temp2));

  return temp1;
}

void stmts_delete(struct list_head *stmts) {
  struct list_head *node;
  if (!list_empty(stmts)) {
    list_for_each(node, stmts) {
      if (node->prev != stmts)
        stmt_delete(stmt_ent(node->prev));
    }
    stmt_delete(stmt_ent(node->prev));
  }
  free(stmts);
}
