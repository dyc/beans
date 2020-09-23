#include <stddef.h>
#include <stdint.h>

static const int VGA_TEXT_START = 0xB8000;
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

void loadk(size_t mmap_entries, struct mmap_entry *mmap) {
  (void)mmap_entries;
  (void)mmap;
  buffer = (uint16_t *)VGA_TEXT_START;
  for (size_t y = 0; y < VGA_HEIGHT; ++y) {
    for (size_t x = 0; x < VGA_WIDTH; ++x) {
      buffer[buffer_i(x, y)] = vga_cell(' ');
    }
  }
  for (size_t i = 0; i < mmap_entries; ++i) {
    buffer[buffer_i(0, i)] = vga_cell('h');
    buffer[buffer_i(1, i)] = vga_cell('o');
    buffer[buffer_i(2, i)] = vga_cell('w');
    buffer[buffer_i(3, i)] = vga_cell('d');
    buffer[buffer_i(4, i)] = vga_cell('y');
  }
  while (1)
    ;
}
