#include <string.h>
#include <stdlib.h>

#define _UTILS_CHECKMEM_C
#include "utils/checkmem.h"
#include "utils/errhand.h"

#define CHK(out) do {         \
  void* __val = (void*)(out); \
  if (!__val) {               \
    gfatal("Out of memory!"); \
  }                           \
  return __val;               \
} while(0)

void* malloc_c(size_t size) {
  CHK(malloc(size));
}
void* calloc_c(size_t nmemb, size_t size) {
  CHK(calloc(nmemb, size));
}
void* realloc_c(void *ptr, size_t size) {
  CHK(realloc(ptr, size));
}
char* strdup_c(const char *s) {
  CHK(strdup(s));
}
void* memdup_c(const void* src, size_t len) {
  void* dst = malloc_c(len);
  memcpy(dst, src, len);
  return dst;
}

