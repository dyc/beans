#ifndef BEANS_BOOT_UTIL_H_
#define BEANS_BOOT_UTIL_H_

#include <stdarg.h>
#include <stddef.h>

#include "io.h"

// todo: these
long long __divdi3(long long a, long long b) {
  (void)a;
  (void)b;
  return 0;
}

long long __moddi3(long long a, long long b) {
  (void)a;
  (void)b;
  return 0;
}

static size_t decwidth(int value) {
  if (value < 0) {
    value *= -1;
  }

  size_t width = 0;
  do {
    ++width;
    value = value / 10;
  } while (value > 0);
  return width;
}

static size_t declwidth(long value) {
  if (value < 0) {
    value *= -1;
  }

  size_t width = 0;
  do {
    ++width;
    value = value / 10;
  } while (value > 0);
  return width;
}

static size_t decllwidth(long long value) {
  if (value < 0) {
    value *= -1;
  }

  size_t width = 0;
  do {
    ++width;
    value = value / 10;
  } while (value > 0);
  return width;
}

static int printdec(char *out, int value) {
  char *stop = out;
  if (value < 0) {
    *stop = '-';
    ++stop;
  }

  char *start = stop + decwidth(value) - 1;
  for (char *o = start; o >= stop; --o) {
    *o = (value % 10) + '0';
    value /= 10;
  }
  return start - out + 1;
}

static int printdecl(char *out, long value) {
  char *stop = out;
  if (value < 0) {
    *stop = '-';
    ++stop;
  }

  char *start = stop + declwidth(value) - 1;
  for (char *o = start; o >= stop; --o) {
    *o = (value % 10) + '0';
    value /= 10;
  }
  return start - out + 1;
}

static int printdecll(char *out, long long value) {
  char *stop = out;
  if (value < 0) {
    *stop = '-';
    ++stop;
  }

  char *start = stop + decllwidth(value) - 1;
  for (char *o = start; o >= stop; --o) {
    *o = (value % 10) + '0';
    value /= 10;
  }
  return start - out + 1;
}

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len]) {
    len++;
  }
  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list arg_p;
  va_start(arg_p, fmt);

  char *o = out;
  for (const char *f = fmt; *f;) {
    if (*f != '%') {
      *o = *f;
      ++o;
      ++f;
      continue;
    }

    ++f;

    int length_modifier = 0;
    while (1) {
      if ('l' == *f) {
        length_modifier += 1;
        ++f;
      } else {
        break;
      }
    }

    switch (*f) {
    case 'd': {
      if (0 == length_modifier) {
        o += printdec(o, va_arg(arg_p, int));
      } else if (1 == length_modifier) {
        o += printdecl(o, va_arg(arg_p, long));
      } else if (2 == length_modifier) {
        o += printdecll(o, va_arg(arg_p, long long));
      }
      ++f;
      break;
    }
    default: {
      break;
    }
    }
  }
  va_end(arg_p);

  *o = '\0';
  return o - out;
}

#endif
