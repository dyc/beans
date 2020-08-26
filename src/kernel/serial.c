#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/libc.h>
#include <kernel/serial.h>
#include <sys/io.h>

// useful: https://www.activexperts.com/serial-port-component/tutorials/uart/
static uint8_t ENABLE_SET_DIVISOR = 0x80;
// (disable divisor mode) set data length to 8 bits,
// 1 stop bit, no parity bit, break control disabled
static uint8_t DEFAULT_LINE_CONFIG = 0x03;
// enable 14 byte FIFO and clear it
static uint8_t DEFAULT_FIFO_CONFIG = 0xC7;
// set write-readiness bits
// we're write-only for now so don't need interrupts enabled
static uint8_t DEFAULT_MODEM_CONFIG = 0x03;
static uint8_t TRANSMIT_FIFO_EMPTY = 0x20;

static inline short int data_reg(enum serial_port p) {
  return p;
}

static inline short intenable_reg(enum serial_port p) {
  return p + 1;
}

static inline short int fifocontrol_reg(enum serial_port p) {
  return p + 2;
}

static inline short int linecontrol_reg(enum serial_port p) {
  return p + 3;
}

static inline short int modemcontrol_reg(enum serial_port p) {
  return p + 4;
}

static inline short int linestatus_reg(enum serial_port p) {
  return p + 5;
}

bool transmit_empty(enum serial_port p) {
  return inb(linestatus_reg(p)) & TRANSMIT_FIFO_EMPTY;
}

void transmit(enum serial_port p, char c) {
  while (!transmit_empty(p));
  outb(data_reg(p), c);
}

void serial_enable(enum serial_port p) {
  outb(intenable_reg(p), 0x00);
  // 115200 bps
  serial_set_baud(p, 0x01);
  outb(linecontrol_reg(p), DEFAULT_LINE_CONFIG);
  outb(fifocontrol_reg(p), DEFAULT_FIFO_CONFIG);
  outb(modemcontrol_reg(p), DEFAULT_MODEM_CONFIG);
  outb(intenable_reg(p), 0x01);
}

void serial_set_baud(enum serial_port p, unsigned short divisor) {
  uint8_t prevline = inb(linecontrol_reg(p));
  outb(linecontrol_reg(p), ENABLE_SET_DIVISOR);
  outb(data_reg(p), (divisor >> 8) & 0xFF);
  outb(data_reg(p), divisor & 0xFF);
  // seems like folks usually write something like
  // DEFAULT_LINE_CONFIG in lieu of prevline
  outb(linecontrol_reg(p), prevline);
}

void serial_write(enum serial_port p, const char* s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len; ++i) {
    transmit(p, s[i]);
  }
}
