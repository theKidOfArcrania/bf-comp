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

#ifndef _COMPILE_AST_H
#define _COMPILE_AST_H

#include "utils/loc.h"
#include "utils/cstr.h"
#include "utils/listdef.h"

#define stmt_ent(ptr) list_entry((ptr), struct stmt, stmts)

#define s_add(s, stmt_fn, ...) list_add(&stmt_fn(__VA_ARGS__)->stmts, (s))


enum stmt_types {
  STMT_LITERAL, STMT_PUSHVAR, STMT_POPVAR, STMT_DELVAR, STMT_ATVAR,
  STMT_PUSHCTX, STMT_POPCTX
};

typedef struct stmt {
  int type;
  struct list_head stmts;
  YYLTYPE loc;
  union {
    cstr *literal;
    char *iden;
    struct {
      char *from;
      char *to;
    } iden_tup;
    void *ptr;
  } d;

} stmt;

typedef struct var {
  char *iden;
  char is_temp;
} var;

stmt *stmt_literal(const YYLTYPE* loc, cstr *lit);
stmt *stmt_pushvar(const YYLTYPE* loc, char *iden);
stmt *stmt_delvar(const YYLTYPE* loc, char *iden);
stmt *stmt_popvar(const YYLTYPE* loc, char *iden);
stmt *stmt_atvar(const YYLTYPE* loc, char *iden);
stmt *stmt_pushctx(const YYLTYPE* loc);
stmt *stmt_popctx(const YYLTYPE* loc); // TODO: must restore variable stack and tape ptr

void s_add_literal(struct list_head *stmts, const YYLTYPE* loc, char *iden);

char *stmts_tmpvar(struct list_head *stmts, const YYLTYPE* loc, int is_zero);
char *stmts_var_maketemp(struct list_head *stmts, const YYLTYPE* loc, const char *iden);
#endif
