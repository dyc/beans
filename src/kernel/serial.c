#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/libc.h>
#include <kernel/serial.h>
#include <sys/io.h>

static uint8_t ENABLE_SET_DIVISOR = 0x80;
// (with divisor mode disabled) set data length to 8 bits,
// 1 stop bit, no parity bit, break control disabled
static uint8_t DEFAULT_LINE_CONFIG = 0x03;
// enable 14 byte FIFO and clear it
static uint8_t DEFAULT_FIFO_CONFIG = 0xC7;
// set write-readiness bits
// we're write-only for now so don't need interrupts enabled
static uint8_t DEFAULT_MODEM_CONFIG = 0x03;
static uint8_t TRANSMIT_FIFO_EMPTY = 0x20;

static inline short int data_port(enum serial_port p) {
  return p;
}

static inline short int fifo_port(enum serial_port p) {
  return p + 2;
}

static inline short int line_port(enum serial_port p) {
  return p + 3;
}

static inline short int modem_port(enum serial_port p) {
  return p + 4;
}

static inline short int status_port(enum serial_port p) {
  return p + 5;
}

bool transmit_empty(enum serial_port p) {
  return inb(status_port(p)) & TRANSMIT_FIFO_EMPTY;
}

void transmit(enum serial_port p, char c) {
  while (!transmit_empty(p));
  outb(data_port(p), c);
}

void serial_enable(enum serial_port p) {
  // 115200 bps
  serial_set_baud(p, 0x01);
  outb(line_port(p), DEFAULT_LINE_CONFIG);
  outb(fifo_port(p), DEFAULT_FIFO_CONFIG);
  outb(modem_port(p), DEFAULT_MODEM_CONFIG);
}

void serial_set_baud(enum serial_port p, unsigned short divisor) {
  uint8_t prev = inb(line_port(p));
  outb(line_port(p), ENABLE_SET_DIVISOR);
  outb(data_port(p), (divisor >> 8) & 0x00FF);
  outb(data_port(p), divisor & 0x00FF);
  outb(line_port(p), prev);
}

void serial_write(enum serial_port p, const char* s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; ++i) {
    transmit(p, s[i]);
  }
}
