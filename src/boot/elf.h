#pragma once

#include <stdint.h>

#define ELFI_N 16
#define ELFI_MAGIC0 0x7f
#define ELFI_MAGIC1 'E'
#define ELFI_MAGIC2 'L'
#define ELFI_MAGIC3 'F'

struct elf_header {
  uint8_t ident[ELFI_N];
} __attribute__((packed));
