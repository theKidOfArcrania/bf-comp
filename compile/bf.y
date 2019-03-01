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

%code requires {

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#include "compile/ast.h"
#include "utils/loc.h"
#include "utils/cstr.h"
#include "utils/listdef.h"

typedef struct _IO_FILE FILE;

struct parse_env {
  FILE *infile;
  char *current_file;
  yyscan_t scanner;    
  struct list_head *ast;
};

#define YYLTYPE YYLTYPE

}

%initial-action {
  @$.last_line = @$.first_line = 1;
  @$.last_column = @$.first_column = 0;
  @$.sourceFile = env->current_file;
}

%code {

#define _GNU_SOURCE

#include "compile/bf_lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/list.h"
#include "utils/checkmem.h"
#include "utils/errhand.h"

#define YYMALLOC malloc_c

extern int yylex(YYSTYPE*, YYLTYPE*, yyscan_t);
extern int yyparse(struct parse_env *, yyscan_t);

#define yyerror(ploc, unused, unused2, ...) printMsg(MSG_ERROR, ploc, __VA_ARGS__)

#define YYLLOC_DEFAULT(Cur, Rhs, N) do {      \
  if (N) {                                    \
      LCOPY_BEGIN((Cur), YYRHSLOC(Rhs, 1));   \
      LCOPY_END((Cur), YYRHSLOC(Rhs, N));     \
  } else {                                    \
      LCOPY_END((Cur), YYRHSLOC(Rhs, 0));     \
      (Cur).first_line = (Cur).last_line;     \
      (Cur).first_column = (Cur).last_column; \
  }                                           \
} while (0)

static inline void list_new(struct list_head **phead) {
  INIT_LIST_HEAD(*phead = malloc_c(sizeof(**phead)));
}

}

%locations
%define parse.error verbose
%define api.pure full
%parse-param {struct parse_env *env}
%param {void *scanner}

%union {
  char ch;
  int32_t num;
  char *str;
  struct list_head *stmts;
}

%token T_EOF 0 "end of file"
%token T_IF T_THEN T_ELSE T_END T_WHILE T_DIM T_FOR T_TO T_FROM T_TEMP
%token T_AT T_LET
%token <num> T_NUM
%token <str> T_IDEN T_CODE

%type <stmts> stmts stmt
%type <ch> tmpable

%%

main: stmts {
      env->ast = $1;
    };

stmts: stmts stmt {
      $$ = $1;
      list_splice_tail($2, $$);
      free($2);
     } | %empty {
      list_new(&$$);
     }
     ;

stmt: T_CODE {
      list_new(&$$);
      s_add($$, stmt_literal, &@1, cstr_new_smv($1));
    } | T_IF tmpable T_IDEN T_THEN stmts T_END T_IF {
      list_new(&$$);

      char *var = $3;
      if (!$2)
        var = stmts_var_maketemp($$, &@3, var);

      // if [var] then { [var] = 0; }
      s_add($$, stmt_atvar, &@3, var);
      s_add($$, stmt_literal, &@3, cstr_new_scp("[[-]")); 
      s_add($$, stmt_pushctx, &@3);
      list_splice_tail($5, $$);
      free($5);
      s_add($$, stmt_popctx, &@6);
      s_add($$, stmt_literal, &@6, cstr_new_scp("]"));
    } | T_IF tmpable T_IDEN T_THEN stmts T_ELSE stmts T_END T_IF {
      char *var = $3, *var2;

      list_new(&$$);
      if (!$2)
        var = stmts_var_maketemp($$, &@3, var);

      // [var2] = 1
      var2 = stmts_tmpvar($$, &@3, 1);
      s_add($$, stmt_literal, &@3, cstr_new_scp("+"));

      // if [var] then { [var] = 0; [var2]--; if_stmts; }
      s_add($$, stmt_atvar, &@3, var);
      s_add($$, stmt_literal, &@3, cstr_new_scp("[[-]")); 
      s_add($$, stmt_atvar, &@3, var2);
      s_add($$, stmt_literal, &@3, cstr_new_scp("-")); 
      s_add($$, stmt_pushctx, &@3);
      list_splice_tail($5, $$);
      free($5);
      s_add($$, stmt_popctx, &@6);
      s_add($$, stmt_atvar, &@6, strdup_c(var));
      s_add($$, stmt_literal, &@6, cstr_new_scp("]"));
      s_add($$, stmt_delvar, &@6, strdup_c(var));

      // if [var2] { else_stmts; }
      s_add($$, stmt_atvar, &@6, strdup_c(var2));
      s_add($$, stmt_literal, &@6, cstr_new_scp("["));
      s_add($$, stmt_pushctx, &@6);
      list_splice_tail($7, $$);
      free($7);
      s_add($$, stmt_popctx, &@6);
      s_add($$, stmt_literal, &@8, cstr_new_scp("]"));
      s_add($$, stmt_delvar, &@8, strdup_c(var2));
    } | T_WHILE tmpable T_IDEN stmts T_END T_WHILE {
      char *var = $3;

      list_new(&$$);
      s_add($$, stmt_atvar, &@3, var);
      s_add($$, stmt_literal, &@3, cstr_new_scp("["));
      s_add($$, stmt_pushctx, &@3);
      list_splice_tail($4, $$);
      free($4);
      s_add($$, stmt_popctx, &@5);
      s_add($$, stmt_atvar, &@5, strdup_c(var));
      s_add($$, stmt_literal, &@5, cstr_new_scp("]"));
      if ($2)
        s_add($$, stmt_delvar, &@5, strdup_c(var));

    } | T_FOR T_IDEN T_FROM expr T_TO expr stmts T_END T_FOR {
      lerror(&@1, "For loops not supported yet...");
    } | T_AT T_IDEN {
      list_new(&$$);
      s_add($$, stmt_atvar, &@2, $2);
    } | T_LET stmts T_END T_LET {
      list_new(&$$);
      s_add($$, stmt_pushctx, &@1);
      list_splice_tail($2, $$);
      free($2);
      s_add($$, stmt_popctx, &@1);
    } | T_DIM T_IDEN {
      list_new(&$$);
      s_add($$, stmt_pushvar, &@2, $2);
    } | T_NUM { 
      char *ch;
      int pos = $1 & 0xff; 
      int neg = 256 - pos;
      if (pos < neg) {
        ch = malloc_c(pos + 1);
        memset(ch, '+', pos);
        ch[pos] = 0;
      } else {
        ch = malloc_c(neg + 1);
        memset(ch, '-', neg);
        ch[neg] = 0;
      }

      list_new(&$$);
      s_add($$, stmt_literal, &@1, cstr_new_smv(ch));
    }
    ;

expr: tmpable T_IDEN | T_NUM

tmpable: T_TEMP { $$ = 1; } | %empty { $$ = 0; } ;

%%
