#pragma once

#include <stdint.h>

typedef enum {
  PIC_1 = 0x20,
  PIC_2 = 0xA0,
} pic_port_t;

typedef struct {
  // need to check the order of these
  uint16_t di, si, bp, sp, bx, dx, cx, ax;
  unsigned int error_code, interrupt;
  unsigned int eip, cs, eflags;
} __attribute__((packed)) irq_state_t;

extern void gdt_install();
extern void idt_install();
extern void irq_install();
extern void irq_handler(irq_state_t* s);
