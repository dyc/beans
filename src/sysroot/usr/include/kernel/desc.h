#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
  PIC1 = 0x20,
  PIC2 = 0xA0,
} pic_port_t;

typedef struct {
  // need to check the order of these
  uint16_t di, si, bp, sp, bx, dx, cx, ax;
  unsigned int error_code, interrupt;
  unsigned int eip, cs, eflags;
} __attribute__((packed)) irq_state_t;

typedef int (*irq_handler_t) (irq_state_t* s);

extern void gdt_install();
extern void idt_install();
extern void irq_install();
extern void irq_install_handler(size_t irq, irq_handler_t* handler);
