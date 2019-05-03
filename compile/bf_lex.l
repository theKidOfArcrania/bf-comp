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

//typedef 


#define MAX_STRLEN 4096
#define STRC(c) do { \
 *buffptr++ = (char)(c); \
 if (buffptr >= buff + MAX_STRLEN) { \
   lerror(yylloc, "Reached 4k string limit."); \
   BEGIN(INITIAL); \
   YY_BREAK; \
 } \
} while (0)


#define YY_USER_ACTION updateLocation(yylloc, yytext);

#define yyalloc(size, ctx) malloc_c(size)
#define yyrealloc(buf, size, ctx) realloc_c(buf, size)

static void updateLocation(YYLTYPE *loc, char *tok);

char buff[MAX_STRLEN];
char *buffptr;

%}

%option noyyalloc noyyrealloc noyywrap nounput noinput 8bit caseless
%option reentrant bison-bridge warn bison-locations

%x CMT_LINE CHAR

%%

<CMT_LINE>{
  \n { BEGIN(INITIAL); }
  .  ;
}

<CHAR>{
  [\"\'\\]          { STRC(yytext[1]); }
  \\[0-9]{3}        { STRC(strtol(yytext+1, NULL, 8)); }
  \\0               { STRC(0); }
  \\x[a-fA-F0-9]{2} { STRC(strtol(yytext+2, NULL, 16)); }
  \\x               { lerror(yylloc, "Invalid hexadecimal for 'x' escape."); }
  \\n               { STRC('\n'); }
  \\r               { STRC('\r'); }
  \t|\\t            { STRC('\t'); }
  [ ]               { STRC(' '); }
  \n                { 
    lerror(yylloc, "Unexpected newline in string."); 
    BEGIN(INITIAL); 
  }
  \\.               { 
    lerror(yylloc, "Invalid escape character '%c'.", yytext[1]);
  }
  .                 { STRC(yytext[0]); }
  <<EOF>>           { 
    lerror(yylloc, "Reached end of file while parsing string."); 
    BEGIN(INITIAL);
    return T_EOF;
  }
}

[-]?[0-9]+ { 
  char *end;
  errno = 0;
  long int num = strtol(yytext, &end, 10);
  if (errno || *end) {
    lwarning(yylloc, "number overflowed!");
    errno = 0;
    num = 0;
  } else if ((num % 256) != num) {
    lwarning(yylloc, "number is not in the range -256 to 256.");
    num %= 256;
  }

  yylval->num = num;
  return T_NUM;
}

[']   { BEGIN(CHAR); }

[ \n\t\r] ;

"#"   { BEGIN(CMT_LINE); }

[!][<>] { 
  if (yytext[1] == '<') {
    yylval->num = -1;
  } else {
    yylval->num = 1;
  }
  return T_ARR_SHIFT;
}

[-[\]+<>.,\^]+ { yylval->str = strdup_c(yytext); return T_CODE; }


IF      { return T_IF; }
THEN    { return T_THEN; }
ELSE    { return T_ELSE; }
END     { return T_END; }

AT      { return T_AT; }

WHILE   { return T_WHILE; }

DIM     { return T_DIM; }

FOR     { return T_FOR; }
TO      { return T_TO; }
FROM    { return T_FROM; }

TEMP    { return T_TEMP; }

LET     { return T_LET; }

LIMIT   { return T_LIMIT; }
UNLIMIT { return T_UNLIMIT; }

[A-Za-z_][A-Za-z0-9_]* { 
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
