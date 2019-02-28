#ifndef _COMPILE_AST_H
#define _COMPILE_AST_H

#include "utils/errhand.h"
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

char *stmts_tmpvar(struct list_head *stmts, const YYLTYPE* loc, int is_zero);
char *stmts_var_maketemp(struct list_head *stmts, const YYLTYPE* loc, const char *iden);
#endif
