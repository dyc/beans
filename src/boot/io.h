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
static uint16_t INTENABLE = 0x3F9;
static uint16_t FIFOCONTROL = 0x3FA;
static uint16_t LINECONTROL = 0x3FB;
static uint16_t MODEMCONTROL = 0x3FC;
static uint16_t LINESTATUS = 0x3FD;
static uint8_t ENABLE_SET_DIVISOR = 0x80;
static uint8_t DEFAULT_LINE_CONFIG = 0x03;
static uint8_t DEFAULT_FIFO_CONFIG = 0xC7;
static uint8_t DEFAULT_MODEM_CONFIG = 0x03;
static uint8_t TRANSMIT_FIFO_EMPTY = 0x20;

void serial_enable() {
  outb(INTENABLE, 0x00);
  outb(LINECONTROL, ENABLE_SET_DIVISOR);
  outb(DATA, (0x01 >> 8) & 0xFF);
  outb(DATA, 0x01 & 0xFF);
  outb(LINECONTROL, DEFAULT_LINE_CONFIG);
  outb(FIFOCONTROL, DEFAULT_FIFO_CONFIG);
  outb(MODEMCONTROL, DEFAULT_MODEM_CONFIG);
  outb(INTENABLE, 0x01);
}

void serial_write(const char *s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; ++i) {
    while (!inb(LINESTATUS & TRANSMIT_FIFO_EMPTY))
      ;
    outb(DATA, s[i]);
  }
}

#endif
