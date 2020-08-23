#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/vga.h>

static uint16_t EMPTY_CELL;
static uint16_t* buffer;
size_t row;
size_t col;
enum vga_color fg;
enum vga_color bg;
const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;

static inline uint16_t vga_cell(unsigned char uc, enum vga_color fg, enum vga_color bg) {
  return (uint16_t) uc | (uint16_t) ((fg | bg << 4) << 8);
}

static inline size_t buffer_i(size_t x, size_t y) {
  return y * VGA_WIDTH + x;
}

static size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

static void putentryat(char c, size_t x, size_t y) {
  buffer[buffer_i(x, y)] = vga_cell(c, fg, bg);
}

void scroll() {
  const size_t last_row = VGA_HEIGHT - 1;
  for (size_t y = 0; y < last_row; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t this_row_i = buffer_i(x, y);
      const size_t next_row_i = buffer_i(x, y + 1);
      buffer[this_row_i] = buffer[next_row_i];
    }
  }

  for (size_t x = 0; x < VGA_WIDTH; x++) {
    buffer[buffer_i(x, last_row)] = EMPTY_CELL;
  }

  row = last_row;
  col = 0;
}

static void putchar(char c) {
  if (c == '\n') {
    if (++row == VGA_HEIGHT) {
      scroll();
    }
    col = 0;
    return;
  }
  putentryat(c, col, row);
  if (++col == VGA_WIDTH) {
    col = 0;
    if (++row == VGA_HEIGHT) {
      scroll();
    }
  }
}

void write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    putchar(data[i]);
  }
}

void vga_fg(enum vga_color c) {
  fg = c;
}

void vga_bg(enum vga_color c) {
  bg = c;
}

void vga_writestring(const char* s) {
  write(s, strlen(s));
}

void vga_init() {
  buffer = (uint16_t*) 0xB8000;
  EMPTY_CELL = vga_cell(' ', VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  row = 0;
  col = 0;
  fg = VGA_COLOR_LIGHT_GREY;
  bg = VGA_COLOR_BLACK;

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      buffer[buffer_i(x, y)] = EMPTY_CELL;
    }
  }
}
