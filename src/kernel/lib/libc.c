#include <stddef.h>

#include <kernel/libc.h>

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len]) {
    len++;
  }
  return len;
}
