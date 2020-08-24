#include <kernel/serial.h>
#include <sys/io.h>

static unsigned char ENABLE_SET_DIVISOR = 0x80;
// (with divisor mode disabled) set data length to 8 bits,
// 1 stop bit, no parity bit, break control disabled
static unsigned char DEFAULT_LINE_CONFIG = 0x03;
// enable 14 byte FIFO and clear it
static unsigned char DEFAULT_FIFO_CONFIG = 0xC7;
// set write-readiness bits
// we're write-only for now so don't need interrupts enabled
static unsigned char DEFAULT_MODEM_CONFIG = 0x03;

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

void serial_enable(enum serial_port p) {
  // 115200 bps
  serial_set_baud(p, 0x01);
  outb(line_port(p), DEFAULT_LINE_CONFIG);
  outb(fifo_port(p), DEFAULT_FIFO_CONFIG);
  outb(modem_port(p), DEFAULT_MODEM_CONFIG);
}

void serial_set_baud(enum serial_port p, unsigned short divisor) {
  unsigned char prev = inb(line_port(p));
  outb(line_port(p), ENABLE_SET_DIVISOR);
  outb(data_port(p), (divisor >> 8) & 0x00FF);
  outb(data_port(p), divisor & 0x00FF);
  outb(line_port(p), prev);
}
