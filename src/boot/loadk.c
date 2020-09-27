#include <stddef.h>
#include <stdint.h>

#include "elf.h"

static const uint16_t *VGA_TEXT_START = (uint16_t *)0xB8000;
const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;
static uint16_t *buffer;

static inline uint16_t vga_cell(uint8_t uc) {
  return (uint16_t)uc | (uint16_t)((15 | (0 << 4)) << 8);
}

static inline size_t buffer_i(size_t x, size_t y) { return y * VGA_WIDTH + x; }

struct mmap_entry {
  uint64_t base, length;
  uint32_t type, unused;
} __attribute__((packed));

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

  buffer[buffer_i(0, 0)] = vga_cell(':');
  buffer[buffer_i(1, 0)] = vga_cell('-');
  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (kernel_elf->ident[0] != ELFI_MAGIC0 ||
      kernel_elf->ident[1] != ELFI_MAGIC1 ||
      kernel_elf->ident[2] != ELFI_MAGIC2 ||
      kernel_elf->ident[3] != ELFI_MAGIC3) {
    buffer[buffer_i(2, 0)] = vga_cell('(');
  } else {
    buffer[buffer_i(2, 0)] = vga_cell(')');
  }
}
