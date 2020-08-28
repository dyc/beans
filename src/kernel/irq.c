#include <stddef.h>
#include <stdint.h>

#include <kernel/desc.h>
#include <sys/io.h>

static const uint8_t PIC_ACK = 0x20;
static const uint8_t ICW1_ICW4 = 0x01;
static const uint8_t ICW1_INIT = 0x10;
static const uint8_t PIC1_OFFSET = 0x20;
static const uint8_t PIC2_OFFSET = 0x28;
static const uint8_t ICW4_8086 = 0x01;

extern void irq_handler_start0();
extern void irq_handler_start1();
extern void irq_handler_start2();
extern void irq_handler_start3();
extern void irq_handler_start4();
extern void irq_handler_start5();
extern void irq_handler_start6();
extern void irq_handler_start7();
extern void irq_handler_start8();
extern void irq_handler_start9();
extern void irq_handler_start10();
extern void irq_handler_start11();
extern void irq_handler_start12();
extern void irq_handler_start13();
extern void irq_handler_start14();
extern void irq_handler_start15();

static __attribute__((used)) void (*irqs[16]) (void) = {
  irq_handler_start0,
  irq_handler_start1,
  irq_handler_start2,
  irq_handler_start3,
  irq_handler_start4,
  irq_handler_start5,
  irq_handler_start6,
  irq_handler_start7,
  irq_handler_start8,
  irq_handler_start9,
  irq_handler_start10,
  irq_handler_start11,
  irq_handler_start12,
  irq_handler_start13,
  irq_handler_start14,
  irq_handler_start15,
};
static int (*irq_handlers[16])(irq_state_t* s);

static inline void disable_int() {
  asm volatile("cli");
}

static inline void enable_int() {
  asm volatile("sti");
}

static inline uint8_t pic_command(pic_port_t p) {
  return p;
}

static inline uint8_t pic_data(pic_port_t p) {
  return p + 1;
}

static void pic_remap() {
  uint8_t old_p1 = inb(pic_data(PIC1));
  uint8_t old_p2 = inb(pic_data(PIC2));

  outb(pic_command(PIC1), ICW1_ICW4 | ICW1_INIT);
  iowait();
  outb(pic_command(PIC2), ICW1_ICW4 | ICW1_INIT);
  iowait();

  outb(pic_data(PIC1), PIC1_OFFSET);
  iowait();
  outb(pic_data(PIC2), PIC2_OFFSET);
  iowait();

  // set up parent (PIC1) and child (PIC2)
  outb(pic_data(PIC1), 0x04);
  iowait();
  outb(pic_data(PIC2), 0x02);
  iowait();

  // request 8086 mode
  outb(pic_data(PIC1), ICW4_8086);
  iowait();
  outb(pic_data(PIC2), ICW4_8086);
  iowait();

  outb(pic_data(PIC1), old_p1);
  outb(pic_data(PIC2), old_p2);
}

void c_irq_handler(irq_state_t* s) {
  disable_int();
  if (s->interrupt >= PIC1_OFFSET && s->interrupt <= (PIC2_OFFSET + 8)) {
    if (!irq_handlers[s->interrupt - PIC1_OFFSET](s)) {
      // our handler failed
      pic_ack(s->interrupt);
    }
  }
  enable_int();
}

void pic_ack(size_t irq) {
  outb(pic_command(PIC1), PIC_ACK);
  if (irq >= 8) {
    outb(pic_command(PIC2), PIC_ACK);
  }
}

void irq_install() {
  pic_remap();
  size_t n = sizeof(irqs) / sizeof(irqs[0]);
  for (size_t i = 0; i < n; ++i) {
    // flags should be
    // static const uint8_t INT_FLAGS =0x8E;
    // see https://wiki.osdev.org/Interrupt_Descriptor_Table
    idt_set_gate(
      PIC1_OFFSET + i, // gate
      (uint32_t) &irqs[i],         // offset
      // check this
      0x80,            // selector
      1,               // present
      0,               // dpl
      0,               // segment
      0xF              // type
    );
  }
}

void irq_install_handler(size_t irq, irq_handler_t handler) {
  disable_int();
  irq_handlers[irq] = handler;
  enable_int();
}
