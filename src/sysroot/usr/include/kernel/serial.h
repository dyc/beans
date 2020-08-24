#pragma once

enum serial_port {
  SERIAL_PORT_COM1 = 0x3F8,
  SERIAL_PORT_COM2 = 0x2F8,
  SERIAL_PORT_COM3 = 0x3E8,
  SERIAL_PORT_COM4 = 0x2E8,
};

extern void serial_enable(enum serial_port p);
extern void serial_set_baud(enum serial_port p, unsigned short divisor);
