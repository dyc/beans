#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "elf.h"

#define ERROR_HANG error(__LINE__);

static const uint16_t *VGA_TEXT_START = (uint16_t *)0xB8000;
const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;
static uint16_t *buffer;
struct mmap_entry {
  uint64_t base, length;
  uint32_t type, unused;
} __attribute__((packed));

static inline uint16_t vga_cell(uint8_t uc) {
  return (uint16_t)uc | (uint16_t)((15 | (0 << 4)) << 8);
}

static inline size_t buffer_i(size_t x, size_t y) { return y * VGA_WIDTH + x; }

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

static int sprintf(char *out, const char *fmt, ...) {
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

static void error(size_t line_num) {
  char buf[8];
  sprintf(buf, "err: %d", line_num);
  for (size_t i = 0; i < sizeof(buf); ++i) {
    buffer[buffer_i(i, 0)] = vga_cell(buf[i]);
  }
  while (1)
    ;
}

static void kmain() {
  buffer[buffer_i(0, 0)] = vga_cell(':');
  buffer[buffer_i(1, 0)] = vga_cell('-');
  buffer[buffer_i(2, 0)] = vga_cell(')');
  while (1)
    ;
}

// todo: ...is there another way to do this?
void loadk(size_t mmap_entries, struct mmap_entry *mmap, uint32_t *kernel,
           uint32_t *initrd) __attribute__((section(".text.loadk")));
void loadk(size_t mmap_entries, struct mmap_entry *mmap, uint32_t *kernel,
           uint32_t *initrd) {
  (void)mmap_entries;
  (void)mmap;
  (void)initrd;
  buffer = (uint16_t *)VGA_TEXT_START;
  for (size_t y = 0; y < VGA_HEIGHT; ++y) {
    for (size_t x = 0; x < VGA_WIDTH; ++x) {
      buffer[buffer_i(x, y)] = vga_cell(' ');
    }
  }

  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (kernel_elf->ident[0] != ELF_IDENT_MAGIC0 ||
      kernel_elf->ident[1] != ELF_IDENT_MAGIC1 ||
      kernel_elf->ident[2] != ELF_IDENT_MAGIC2 ||
      kernel_elf->ident[3] != ELF_IDENT_MAGIC3) {
    ERROR_HANG;
  }

  if (kernel_elf->ident[4] != ELF_IDENT_32BIT) {
    ERROR_HANG;
  }
  if (kernel_elf->ident[5] != ELF_IDENT_L_ENDIAN) {
    ERROR_HANG;
  }
  if (kernel_elf->isa != ELF_ISA_X86) {
    ERROR_HANG;
  }

  kmain();
}
