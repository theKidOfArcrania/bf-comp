#ifndef _UTILS_CSTR_H
#define _UTILS_CSTR_H

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

#include <stddef.h>


typedef struct cstr {
  char *str;
  size_t cap;
  size_t length;
} cstr;

cstr *cstr_new();
cstr *cstr_new_i(size_t);
cstr *cstr_new_scp(const char *);
cstr *cstr_new_smv(char *);

void cstr_append_c(cstr *, char);
void cstr_append_carr(cstr *, const char*, size_t);
void cstr_append_s(cstr *, const char *);
void cstr_append_str(cstr *, const cstr *);

int cstr_insert_c(cstr *, size_t, char);
int cstr_insert_carr(cstr *, size_t, const char *, size_t);
int cstr_insert_s(cstr *, size_t, const char *);
int cstr_insert_str(cstr *, size_t, const cstr *);

void cstr_delete(cstr *);


#endif
