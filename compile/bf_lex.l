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

%{

#include <errno.h>
#include <string.h>


#include "compile/bf.h"
#include "utils/checkmem.h"
#include "utils/errhand.h"

#define YY_USER_ACTION updateLocation(yylloc, yytext);

#define yyalloc(size, ctx) malloc_c(size)
#define yyrealloc(buf, size, ctx) realloc_c(buf, size)

static void updateLocation(YYLTYPE *loc, char *tok);

%}

%option noyyalloc noyyrealloc noyywrap nounput noinput 8bit caseless
%option reentrant bison-bridge warn bison-locations

%x CMT_LINE

%%

<CMT_LINE>{
  \n { BEGIN(INITIAL); }
  .  ;
}

[-]?[0-9]+ { 
  char *end;
  errno = 0;
  long int num = strtol(yytext, &end, 10);
  if (errno || *end) {
    lwarning(yylloc, "Number overflowed!");
    num = 0;
  } else if ((num % 256) != num) {
    lwarning(yylloc, "Number is not in the range -256 to 256.");
    num %= 256;
  }

  yylval->num = num;
  return T_NUM;
}

[ \n\t\r] ;

"#"   { BEGIN(CMT_LINE); }

[-+<>.,]+ { yylval->str = strdup_c(yytext); return T_CODE; }


IF    { return T_IF; }
THEN  { return T_THEN; }
ELSE  { return T_ELSE; }
END   { return T_END; }

AT    { return T_AT; }

WHILE { return T_WHILE; }

DIM   { return T_DIM; }

FOR   { return T_FOR; }
TO    { return T_TO; }
FROM  { return T_FROM; }

TEMP  { return T_TEMP; }

LET   { return T_LET; }

[A-Za-z_][A-Za-z0-9]* { 
  yylval->str = strdup_c(yytext);
  return T_IDEN; 
}

.     { lerror(yylloc, "Invalid character `%c'.", yytext[0]); }

%%

void updateLocation(YYLTYPE *loc, char *tok) {
  int linenum = loc->first_line = loc->last_line;
  int colnum  = loc->first_column = loc->last_column + 1;
  for (char* s = tok; *s; s++) {
    if (*s == '\n') {
      colnum = 1;
      linenum++;
    } else {
      colnum++;
      if (*s == '\t')
        colnum = (colnum - 1 + 7) / 8 * 8 + 1;
    }
  }
  loc->last_line   = linenum;
  loc->last_column = colnum - 1;
//  printf("%d:%d to %d:%d\n", loc->first_line, loc->first_column,
//    loc->last_line, loc->last_column);
}
