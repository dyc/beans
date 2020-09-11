#include <stdarg.h>
#include <stddef.h>

#include <kernel/printf.h>

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
    switch (*f) {
    case 'd': {
      o += printdec(o, va_arg(arg_p, int));
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
