#ifndef BEANS_KERNEL_LIBC_H_
#define BEANS_KERNEL_LIBC_H_

#include <stddef.h>

extern void *__attribute__((malloc)) malloc(size_t size);
extern void memcpy(void *dest, const void *src, size_t n);
extern void memset(void *dest, int c, size_t n);
extern size_t strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);

#endif
