#ifndef BEANS_SYS_IO_H_
#define BEANS_SYS_IO_H_

#include <stdint.h>

extern void outb(unsigned short port, uint8_t data);
extern uint8_t inb(unsigned short port);
extern void iowait();

#endif
