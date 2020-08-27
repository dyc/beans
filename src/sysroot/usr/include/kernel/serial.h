#pragma once

typedef enum {
  SERIAL_PORT_COM1 = 0x3F8,
  SERIAL_PORT_COM2 = 0x2F8,
  SERIAL_PORT_COM3 = 0x3E8,
  SERIAL_PORT_COM4 = 0x2E8,
} serial_port_t;

extern void serial_enable(serial_port_t p);
extern void serial_set_baud(serial_port_t p, unsigned short divisor);
extern void serial_write(serial_port_t p, const char* s);
