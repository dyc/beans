#ifndef BEANS_BOOT_IO_H_
#define BEANS_BOOT_IO_H_

#include <stddef.h>
#include <stdint.h>

#include "util.h"

static inline uint8_t inb(uint16_t port) {
  uint8_t value;
  asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
  return value;
}

static inline void outb(uint16_t port, uint8_t data) {
  asm volatile("outb %1, %0" ::"dN"(port), "a"(data));
}

static uint16_t DATA = 0x3F8;
static uint8_t ENABLE_SET_DIVISOR = 0x80;
static uint8_t TRANSMIT_FIFO_EMPTY = 0x20;

void serial_enable() {
  outb(DATA + 1, 0x00);
  outb(DATA + 3, ENABLE_SET_DIVISOR);
  outb(DATA, 0x01);
  outb(DATA + 1, 0x00);
  outb(DATA + 3, 0x03);
  outb(DATA + 2, 0xc7);
  outb(DATA + 4, 0x03);
  outb(DATA + 1, 0x01);
}

void serial_write(const char *s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; ++i) {
    while (!(inb(DATA + 5) & TRANSMIT_FIFO_EMPTY))
      ;
    outb(DATA, s[i]);
  }
}

#endif
