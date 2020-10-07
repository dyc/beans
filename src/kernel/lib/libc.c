#include <stddef.h>
#include <stdint.h>

#include <kernel/libc.h>

void memcpy(void *dest, const void *src, size_t n) {
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;
  for (size_t i = 0; i < n; ++i) {
    d[i] = s[i];
  }
}

void memset(void *dest, int c, size_t n) {
  unsigned char *p = (unsigned char *)dest;
  for (size_t i = 0; i < n; ++i) {
    p[i] = (unsigned char)c;
  }
}

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len]) {
    len++;
  }
  return len;
}
