#ifndef _UTILS_CHECKMEM_H
#define _UTILS_CHECKMEM_H

#ifdef __cplusplus
extern "C" 
#endif

#include <sys/types.h>

#ifndef _UTILS_CHECKMEM_C

#define BUILD_ERROR $%^%$&*
#define malloc BUILD_ERROR
#define calloc BUILD_ERROR
#define realloc BUILD_ERROR
#define strdup BUILD_ERROR
#define memdup BUILD_ERROR

#endif



void* malloc_c(size_t size);
void* calloc_c(size_t nmemb, size_t size);
void* realloc_c(void *ptr, size_t size);
char* strdup_c(const char *s);
void* memdup_c(const void *mem, size_t len);

#ifdef __cplusplus
}
#endif

#endif 
