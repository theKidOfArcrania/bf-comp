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
#include <errno.h>

#include "utils/errhand.h"

static char* line = NULL;
static size_t lineLen = 0;

static char* sourceFile = NULL;
static int linenum = 0;
static FILE* src;

static int logCount[4];

static void nextLine() {
  if (src) {
    ssize_t read = getline(&line, &lineLen, src);
    if (errno != 0)
      gerror("getline() failed: %s", strerror(errno));
    line[(read == -1) ? 0 : read - 1] = 0;
  } else if (lineLen) {
    line[0] = 0;
  } 
}

static void updateSourceLocation(const char* newSourceFile, int newLinenum) {
  int scanLines = newLinenum;
  if (!sourceFile || strcmp(sourceFile, newSourceFile)) {
    free(sourceFile);
    sourceFile = strdup(newSourceFile);
  } else if (linenum < newLinenum) {
    scanLines = newLinenum - linenum;
  }
  
  if (scanLines == newLinenum) {
    if (src)
      fclose(src);
    if (!(src = fopen(sourceFile, "r"))) {
      gerror("Cannot open file '%s': %s", sourceFile, strerror(errno));
    }
  } 
  while (newLinenum --> 0) {
    nextLine();
  }
}

int errors() {
  return logCount[MSG_ERROR] + logCount[MSG_FATAL];
}

int warnings() {
  return logCount[MSG_WARNING];
}

int infos() {
  return logCount[MSG_INFO];
}

void printMsg(enum msgType type, const YYLTYPE* loc, const char* fmt, ...) {

  const char* color;
  const char* heading;
  switch (type) {
    case MSG_FATAL: color = cLRD; heading = "fatal error"; break;
    case MSG_ERROR: color = cLRD; heading = "error"; break;
    case MSG_WARNING: color = cPIN; heading = "warning"; break;
    case MSG_INFO: color = cLCY; heading = "info"; break;
    default: return;
  }

  logCount[type]++;

  if (loc) {
    printf(cBRI"%s:%d:%d: %s%s"cRST": ", loc->sourceFile, loc->first_line, 
        loc->first_column, color, heading);
  } else { 
    printf("%s%s"cRST": ", color, heading);
  }

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  puts("");  

  if (loc) {
    //TODO: multiple lines? multiple character token? highlight token?
    updateSourceLocation(loc->sourceFile, loc->first_line);
    puts(line);

    int cols = loc->first_column - 1;
    //FIXME: buffer/stack overflow!!!
    char buff[cols + 20];
    memset(buff, ' ', cols);
    strcpy(buff + cols, cYEL"^"cRST"\n");
    puts(buff);
  }

  if (type == MSG_FATAL) {
    puts("Terminating now.");
    exit(1);
  }
}
