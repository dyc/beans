#include <stddef.h>
#include <stdint.h>

#include <kernel/libc.h>

void *malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  void *ret = NULL;

  return ret;
}

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

int strcmp(const char *s1, const char *s2) {
  unsigned char *p1 = (unsigned char *)s1;
  unsigned char *p2 = (unsigned char *)s2;
  while (1) {
    unsigned char c1 = *(p1++);
    unsigned char c2 = *(p2++);
    if (c1 == 0 && c2 == 0) {
      return 0;
    } else if (c1 == c2) {
      continue;
    } else if (c1 < c2) {
      return -1;
    } else {
      return 1;
    }
  }
}
