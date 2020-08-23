#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/vga.h>
#include <sys/io.h>

const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;

static const int VGA_TEXT_START = 0xB8000;
static const short COMMAND_PORT = 0x3D4;
static const short DATA_PORT = 0x3D5;
static const short CURSOR_ENABLE_HIGH = 0x0A;
static const short CURSOR_ENABLE_LOW = 0x0B;
static const short CURSOR_POS_HIGH = 0x0E;
static const short CURSOR_POS_LOW = 0x0F;
static const short CURSOR_DISABLE_BIT = 0x20;
static const short CURSOR_MIN_SCANLINE = 0x00;
static const short CURSOR_MAX_SCANLINE = 0x0F;
static uint16_t EMPTY_CELL;

static uint16_t* buffer;
static size_t row;
static size_t col;
static enum vga_color fg;
static enum vga_color bg;

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

void enable_cursor() {
  outb(COMMAND_PORT, CURSOR_ENABLE_HIGH);
  // TODO: check inb() as well as the 0xC0 and 0xE0 magic nums
  outb(DATA_PORT, (inb(DATA_PORT) & 0xC0) | CURSOR_MIN_SCANLINE);
  outb(COMMAND_PORT, CURSOR_ENABLE_LOW);
  outb(DATA_PORT, (inb(DATA_PORT) & 0xE0) | CURSOR_MAX_SCANLINE);
}

void disable_cursor() {
  outb(COMMAND_PORT, CURSOR_ENABLE_HIGH);
  outb(DATA_PORT, CURSOR_DISABLE_BIT);
}

void move_cursor(size_t x, size_t y) {
  uint16_t i = (uint16_t) buffer_i(x, y);
  outb(COMMAND_PORT, CURSOR_POS_HIGH);
  outb(DATA_PORT, (i >> 8));
  outb(COMMAND_PORT, CURSOR_POS_LOW);
  outb(DATA_PORT, i);
}

void write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    putchar(data[i]);
  }
  move_cursor(col, row);
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
  buffer = (uint16_t*) VGA_TEXT_START;
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

  // test outb() and inb() implmentations
  disable_cursor();
  enable_cursor();
}
