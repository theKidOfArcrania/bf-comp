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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "compile/bf.h"
#include "compile/bf_lex.h"
#include "utils/errhand.h"
#include "utils/checkmem.h"
#include "utils/list.h"

#define min(a, b) ((a) > (b) ? (b) : (a))

FILE *next_file();
void parse_args(int argc, char **argv);

static char **files;
static char *current_file;

int main(int argc, char **argv) {
  yyscan_t scanner;
  FILE *infile;

  parse_args(argc, argv);

  yylex_init(&scanner);


  if (!(infile = next_file()))
    gfatal("no input files!");
  yyset_in(infile, scanner);

  while (1) {
    struct parse_env penv;
    penv.current_file = current_file;
    switch (yyparse(&penv, scanner)) {
      case 1:
        gfatal("exiting because of previous error.");
        __builtin_unreachable();
      case 2:
        gfatal("memory exhausted!");
        __builtin_unreachable();
    }

    fclose(infile);
    yypop_buffer_state(scanner);

    FILE *next = next_file();
    if (!next) break;

    YY_BUFFER_STATE buff = yy_create_buffer(next, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(buff, scanner);
  }
}

void parse_bfc() {}

FILE *next_file() {
  FILE* f = NULL;
  while (!f && *files) {
    const char* file = *(files++);
    if (current_file)
      free(current_file);
    current_file = strdup_c(file);

    int fd = open(file, O_RDONLY);
    if (fd < 0) {
      gerror("Cannot open file '%s': %s", file, strerror(errno));
      continue;
    }

    struct stat type;
    if (fstat(fd, &type)) {
      gerror("fstat() failed: %s", file, strerror(errno));
      continue;
    }
    
    if (!S_ISREG(type.st_mode)) {
      gerror("Not a regular file: '%s'", file);
      continue;
    }

    f = fdopen(fd, "r");
  }
  return f;
}

void usage() {
  fprintf(stderr, 
      "Usage: %1$s [OPTION]... FILE...\n"
      "A high-level compiler for brainf**k.\n"
      "\n"
      "Where options include:\n"
      "  -h   prints this help message and exits.\n"
      "  -o   output file, defaults to a.bf.\n"
      "\n"
      "brainf**k compiler Copyright (C) 2019 theKidOfArcrania\n"
      "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n"
      "\n"
      "Written by theKidOfArcrania\n"
      , program_invocation_short_name);
  exit(2);
}

void parse_args(int argc, char **argv) {
  int opt;
  while ((opt = getopt(argc, argv, "ho:")) != -1) {
    switch (opt) {
      case 'o':
        gwarning("not implemented!");
        break;
      case 'h':
      case '?':
        usage();
        break;
      default:
        gerror("switch error!");
        abort();
    }
  }

  files = argv + min(argc, optind);
}
