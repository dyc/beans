#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <kernel/libc.h>
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

static size_t hexwidth(unsigned int value) {
  size_t width = 0;
  do {
    ++width;
    value >>= 4;
  } while (value > 0);
  return width;
}

static size_t hexlwidth(unsigned long value) {
  size_t width = 0;
  do {
    ++width;
    value >>= 4;
  } while (value > 0);
  return width;
}

// static size_t hexllwidth(unsigned long long value) {
//   size_t width = 0;
//   do {
//     ++width;
//     value >>= 4;
//   } while (value > 0);
//   return width;
// }

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

static int printhex(char *out, unsigned int value) {
  char *stop = out;
  *(stop++) = '0';
  *(stop++) = 'x';

  char *start = stop + hexwidth(value) - 1;
  for (char *o = start; o >= stop; --o) {
    unsigned int r = value % 16;
    if (r > 9) {
      *o = (r - 10) + 'A';
    } else {
      *o = r + '0';
    }
    value >>= 4;
  }
  return start - out + 1;
}

static int printhexl(char *out, unsigned long value, bool prefix) {
  char *stop = out;
  if (prefix) {
    *(stop++) = '0';
    *(stop++) = 'x';
  } else if (value == 0) {
    return 0;
  }

  char *start = stop + hexlwidth(value) - 1;
  for (char *o = start; o >= stop; --o) {
    unsigned long r = value % 16;
    if (r > 9) {
      *o = (r - 10) + 'A';
    } else {
      *o = r + '0';
    }
    value >>= 4;
  }
  return start - out + 1;
}

// todo: proper printhexll
// static int printhexll(char *out, unsigned long long value) {
//   char *stop = out;
//   *(stop++) = '0';
//   *(stop++) = 'x';
//
//   char *start = stop + hexllwidth(value) - 1;
//   for (char *o = start; o >= stop; --o) {
//     unsigned long long r = value % 16;
//     if (r > 9) {
//       *o = (r - 10) + 'A';
//     } else {
//       *o = r + '0';
//     }
//     value >>= 4;
//   }
//   return start - out + 1;
// }

static int printstr(char *out, char *s) {
  size_t n = strlen(s);
  for (size_t i = 0; i < n; ++i) {
    out[i] = s[i];
  }
  return n;
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
      }
      ++f;
      break;
    }
    case 's': {
      o += printstr(o, va_arg(arg_p, char *));
      ++f;
      break;
    }
    case 'x': {
      if (0 == length_modifier) {
        o += printhex(o, va_arg(arg_p, unsigned int));
      } else if (1 == length_modifier) {
        o += printhexl(o, va_arg(arg_p, unsigned long), true);
      } else if (2 == length_modifier) {
        unsigned long long arg = va_arg(arg_p, unsigned long long);
        if (arg > 0xffffffff) {
          o += printhexl(o, arg >> 32, true);
          o += printhexl(o, arg, false);
        } else {
          o += printhexl(o, (unsigned long)arg, true);
        }
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
