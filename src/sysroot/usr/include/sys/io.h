#pragma once

#include <stdint.h>

extern void outb(unsigned short port, uint8_t data);
extern uint8_t inb(unsigned short port);
extern void iowait();
