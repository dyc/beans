#ifndef BEANS_KERNEL_DESC_H_
#define BEANS_KERNEL_DESC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum pic_port {
  PIC1 = 0x20,
  PIC2 = 0xA0,
};

struct irq_state {
  unsigned int gs, fs, es, ds;
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
  unsigned int interrupt, error;
  unsigned int eip, cs, eflags, uresp, ss;
} __attribute__((packed));

extern void gdt_install();
extern void idt_install();
extern void idt_set_gate(size_t gate, uint32_t offset, uint16_t selector,
                         uint8_t flags);
extern void pic_ack(size_t irq);
extern void irq_install();
extern void irq_install_isr(size_t irq, int (*handler)(struct irq_state *));
extern void irq_install_trap(size_t irq, int (*handler)(struct irq_state *));

#endif
