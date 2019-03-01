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
#include "compile/il_parser.h"
#include "utils/errhand.h"
#include "utils/checkmem.h"
#include "utils/list.h"

#define min(a, b) ((a) > (b) ? (b) : (a))

int next_file(struct parse_env *penv);
void parse_args(int argc, char **argv);
int parse_bfc(struct parse_env *penv);

static char **files;

int main(int argc, char **argv) {
  parse_args(argc, argv);

  struct parse_env penv;
  memset(&penv, 0, sizeof(penv));

  while (1) {
    int error_before = errors();
    cstr *s = NULL;

    if (!parse_bfc(&penv))
      break;
    
    if (error_before != errors())
      goto clean;

    dump_il(penv.ast, stdout);
    s = compile_il(penv.ast);
    if (error_before != errors())
      goto clean;
    
    puts(s->str);
    cstr_delete(s);

clean:
    if (s) cstr_delete(s);
    if (penv.ast) stmts_delete(penv.ast);
  }

  if (penv.scanner)
    yylex_destroy(penv.scanner);
}

int parse_bfc(struct parse_env *penv) {
  if (!penv->scanner) {
    yylex_init(&penv->scanner);

    if (!next_file(penv))
      gfatal("no input files");
    yyset_in(penv->infile, penv->scanner);
  } else {
    if (!next_file(penv))
      return 0;
    YY_BUFFER_STATE buff = yy_create_buffer(penv->infile, YY_BUF_SIZE, penv->scanner);
    yy_switch_to_buffer(buff, penv->scanner);
  }

  switch (yyparse(penv, penv->scanner)) {
    case 1:
      return 1;
    case 2:
      gfatal("memory exhausted");
  }

  fclose(penv->infile);
  yypop_buffer_state(penv->scanner);
  return 1;
}

int next_file(struct parse_env *penv) {
  if (penv->current_file)
    free(penv->current_file);
  while (*files) {
    const char* file = *(files++);

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

    penv->current_file = strdup_c(file);
    penv->infile = fdopen(fd, "r");
    return 1;
  }
  return 0;
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
