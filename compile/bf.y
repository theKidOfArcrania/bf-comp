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

#include "compile/ast.h"
#include "utils/errhand.h"
#include "utils/cstr.h"
#include "utils/listdef.h"

#define YYLTYPE YYLTYPE

}

%initial-action {
  @$.last_line = @$.first_line = 1;
  @$.last_column = @$.first_column = 0;
  @$.sourceFile = currentFile;
}

%code {

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/list.h"
#include "utils/checkmem.h"

#define YYMALLOC malloc_c

extern const char *currentFile;

extern int yylex();
extern int yylex();
extern int yyparse();
extern FILE *yyin;

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

}

%locations
%define parse.error verbose

%union {
  char ch;
  int32_t num;
  char *str;
  cstr *cstr;
  var var;
  struct list_head stmts;
}

%token T_EOF 0 "end of file"
%token T_IF T_THEN T_ELSE T_END T_WHILE T_DIM T_FOR T_TO T_FROM T_TEMP
%token T_AT T_LET
%token <num> T_NUM
%token <str> T_IDEN T_CODE

%type <cstr> literals literal
%type <stmts> stmts stmt
%type <var> var

%%

main: stmts {
    };

stmts: stmts stmt {
      INIT_LIST_HEAD(&$$);
      list_splice_tail(&$1, &$$);
     } | %empty {
      INIT_LIST_HEAD(&$$);
     }
     ;

stmt: literals {
      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_literal, &@1, $1);
    } | T_IF var T_THEN stmts T_END T_IF {
      INIT_LIST_HEAD(&$$);

      char *var = $2.iden;
      if (!$2.is_temp)
        var = stmts_var_maketemp(&$$, &@2, var);

      // if [var] then { [var] = 0; }
      s_add(&$$, stmt_atvar, &@2, var);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("[[-]")); 
      s_add(&$$, stmt_pushctx, &@3);
      list_splice_tail(&$4, &$$);
      s_add(&$$, stmt_popctx, &@3);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("]"));
    } | T_IF var T_THEN stmts T_ELSE stmts T_END T_IF {
      char *var = $2.iden, *var2;

      INIT_LIST_HEAD(&$$);
      if (!$2.is_temp)
        var = stmts_var_maketemp(&$$, &@2, var);

      // [var2] = 1
      var2 = stmts_tmpvar(&$$, &@2, 1);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("+"));

      // if [var] then { [var] = 0; [var2]--; if_stmts; }
      s_add(&$$, stmt_atvar, &@2, var);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("[[-]")); 
      s_add(&$$, stmt_atvar, &@2, var2);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("-")); 
      s_add(&$$, stmt_pushctx, &@3);
      list_splice_tail(&$4, &$$);
      s_add(&$$, stmt_popctx, &@3);
      s_add(&$$, stmt_atvar, &@2, strdup_c(var));
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("]"));
      s_add(&$$, stmt_delvar, &@2, strdup_c(var));

      // if [var2] { else_stmts; }
      s_add(&$$, stmt_atvar, &@2, strdup_c(var2));
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("["));
      s_add(&$$, stmt_pushctx, &@5);
      list_splice_tail(&$6, &$$);
      s_add(&$$, stmt_popctx, &@5);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("]"));
      s_add(&$$, stmt_delvar, &@2, strdup_c(var2));
    } | T_WHILE var stmts T_END T_WHILE {
      char *var = $2.iden;

      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_atvar, &@2, var);
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("["));
      s_add(&$$, stmt_pushctx, &@3);
      list_splice_tail(&$3, &$$);
      s_add(&$$, stmt_popctx, &@3);
      s_add(&$$, stmt_atvar, &@2, strdup_c(var));
      s_add(&$$, stmt_literal, &@2, cstr_new_scp("]"));
      if ($2.is_temp)
        s_add(&$$, stmt_delvar, &@2, strdup_c(var));

    } | T_FOR T_IDEN T_FROM expr T_TO expr stmts T_END T_FOR {
      yyerror("For loops not supported yet...");
    } | T_AT T_IDEN {
      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_atvar, &@2, $2);
    } | T_LET stmts T_END T_LET {
      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_pushctx, &@1);
      list_splice_tail(&$2, &$$);
      s_add(&$$, stmt_popctx, &@1);
    } | T_DIM T_IDEN {
      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_pushvar, &@2, $2);
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

      INIT_LIST_HEAD(&$$);
      s_add(&$$, stmt_literal, &@1, cstr_new_smv(ch));
    }
    ;

expr: var | T_NUM

var: T_IDEN { $$ = (var){.iden = $1, .is_temp = 0}; }
   | T_TEMP T_IDEN { $$ = (var){.iden = $2, .is_temp = 1}; }
   ;

literals: literals literal {
          cstr_append_str($1, $2);
          cstr_delete($2);
          $$ = $1;
          strdup_c("");
        } | %empty { $$ = cstr_new(); }
        ;

literal: T_CODE { $$ = cstr_new_smv($1); }
       ;

%%
