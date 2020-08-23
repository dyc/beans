#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/fb.h>

static uint16_t EMPTY_CELL;
static uint16_t* buffer;
size_t row;
size_t col;
enum fb_color fg;
enum fb_color bg;
const size_t FB_WIDTH = 80;
const size_t FB_HEIGHT = 25;

static inline uint16_t fb_cell(unsigned char uc, enum fb_color fg, enum fb_color bg) {
  return (uint16_t) uc | (uint16_t) ((fg | bg << 4) << 8);
}

static inline size_t buffer_i(size_t x, size_t y) {
  return y * FB_WIDTH + x;
}

static size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

static void putentryat(char c, size_t x, size_t y) {
  buffer[buffer_i(x, y)] = fb_cell(c, fg, bg);
}

void scroll() {
  const size_t last_row = FB_HEIGHT - 1;
  for (size_t y = 0; y < last_row; y++) {
    for (size_t x = 0; x < FB_WIDTH; x++) {
      const size_t this_row_i = buffer_i(x, y);
      const size_t next_row_i = buffer_i(x, y + 1);
      buffer[this_row_i] = buffer[next_row_i];
    }
  }

  for (size_t x = 0; x < FB_WIDTH; x++) {
    buffer[buffer_i(x, last_row)] = EMPTY_CELL;
  }

  row = last_row;
  col = 0;
}

static void putchar(char c) {
  if (c == '\n') {
    if (++row == FB_HEIGHT) {
      scroll();
    }
    col = 0;
    return;
  }
  putentryat(c, col, row);
  if (++col == FB_WIDTH) {
    col = 0;
    if (++row == FB_HEIGHT) {
      scroll();
    }
  }
}

void write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    putchar(data[i]);
  }
}

void fb_fg(enum fb_color c) {
  fg = c;
}

void fb_bg(enum fb_color c) {
  bg = c;
}

void fb_writestring(const char* s) {
  write(s, strlen(s));
}

void fb_init() {
  buffer = (uint16_t*) 0xB8000;
  EMPTY_CELL = fb_cell(' ', FB_COLOR_LIGHT_GREY, FB_COLOR_BLACK);
  row = 0;
  col = 0;
  fg = FB_COLOR_LIGHT_GREY;
  bg = FB_COLOR_BLACK;

  for (size_t y = 0; y < FB_HEIGHT; y++) {
    for (size_t x = 0; x < FB_WIDTH; x++) {
      buffer[buffer_i(x, y)] = EMPTY_CELL;
    }
  }
}
