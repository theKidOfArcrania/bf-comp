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

#ifndef _UTILS_ERRHAND_H
#define _UTILS_ERRHAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>


#define USE_COLOR

// Terminal colors
#ifdef USE_COLOR

#  define cBLK "\x1b[0;30m"
#  define cRED "\x1b[0;31m"
#  define cGRN "\x1b[0;32m"
#  define cBRN "\x1b[0;33m"
#  define cBLU "\x1b[0;34m"
#  define cMGN "\x1b[0;35m"
#  define cCYA "\x1b[0;36m"
#  define cLGR "\x1b[0;37m"
#  define cGRA "\x1b[1;30m"
#  define cLRD "\x1b[1;31m"
#  define cLGN "\x1b[1;32m"
#  define cYEL "\x1b[1;33m"
#  define cLBL "\x1b[1;34m"
#  define cPIN "\x1b[1;35m"
#  define cLCY "\x1b[1;36m"
#  define cBRI "\x1b[1;37m"
#  define cRST "\x1b[0m"

#  define bgBLK "\x1b[40m"
#  define bgRED "\x1b[41m"
#  define bgGRN "\x1b[42m"
#  define bgBRN "\x1b[43m"
#  define bgBLU "\x1b[44m"
#  define bgMGN "\x1b[45m"
#  define bgCYA "\x1b[46m"
#  define bgLGR "\x1b[47m"
#  define bgGRA "\x1b[100m"
#  define bgLRD "\x1b[101m"
#  define bgLGN "\x1b[102m"
#  define bgYEL "\x1b[103m"
#  define bgLBL "\x1b[104m"
#  define bgPIN "\x1b[105m"
#  define bgLCY "\x1b[106m"
#  define bgBRI "\x1b[107m"

#else

#  define cBLK ""
#  define cRED ""
#  define cGRN ""
#  define cBRN ""
#  define cBLU ""
#  define cMGN ""
#  define cCYA ""
#  define cLGR ""
#  define cGRA ""
#  define cLRD ""
#  define cLGN ""
#  define cYEL ""
#  define cLBL ""
#  define cPIN ""
#  define cLCY ""
#  define cBRI ""
#  define cRST ""

#  define bgBLK ""
#  define bgRED ""
#  define bgGRN ""
#  define bgBRN ""
#  define bgBLU ""
#  define bgMGN ""
#  define bgCYA ""
#  define bgLGR ""
#  define bgGRA ""
#  define bgLRD ""
#  define bgLGN ""
#  define bgYEL ""
#  define bgLBL ""
#  define bgPIN ""
#  define bgLCY ""
#  define bgBRI ""

#endif // USE_COLOR

typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  const char* sourceFile;
} YYLTYPE;

#define LCOPY_BEGIN(dst, src) do {          \
  const YYLTYPE* __src = &(src);            \
  (dst).first_line = __src->first_line;     \
  (dst).first_column = __src->first_column; \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)
#define LCOPY_END(dst, src) do {            \
  const YYLTYPE* __src = &(src);            \
  (dst).last_line = __src->last_line;       \
  (dst).last_column = __src->last_column;   \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)
#define LCOPY(dst, src) do {                \
  const YYLTYPE* __src = &(src);            \
  (dst).first_line = __src->first_line;     \
  (dst).first_column = __src->first_column; \
  (dst).last_line = __src->last_line;       \
  (dst).last_column = __src->last_column;   \
  (dst).sourceFile = __src->sourceFile;     \
} while(0)

extern YYLTYPE yylloc;

enum msgType {
  MSG_FATAL, MSG_ERROR, MSG_WARNING, MSG_INFO
};

void abort();

int errors();
int warnings();
int infos();

#define yyerror(...) printMsg(MSG_ERROR, &(yylloc), __VA_ARGS__)
#define lfatal(loc, ...) printMsg(MSG_FATAL, &(loc), __VA_ARGS__)
#define lerror(loc, ...) printMsg(MSG_ERROR, &(loc), __VA_ARGS__)
#define lwarning(loc, ...) printMsg(MSG_WARNING, &(loc), __VA_ARGS__)
#define linfo(loc, ...) printMsg(MSG_INFO, &(loc), __VA_ARGS__)
#define gfatal(...) printMsg(MSG_FATAL, 0, __VA_ARGS__)
#define gerror(...) printMsg(MSG_ERROR, 0, __VA_ARGS__)
#define gwarning(...) printMsg(MSG_WARNING, 0, __VA_ARGS__)
#define ginfo(...) printMsg(MSG_INFO, 0, __VA_ARGS__)
#define ASSERT(expr) do {                                            \
  if (!(expr)) {                                                     \
    gerror("assertion failed: %s:%s in %s(): `%s` failed", __FILE__, \
        __LINE__, __func__, #expr);                                  \
    abort();                                                         \
  }                                                                  \
} while(0)

void printMsg(enum msgType type, const YYLTYPE* loc, const char* fmt, ...);
#ifdef __cplusplus
}
#endif

#endif // _ERRHAND_H
