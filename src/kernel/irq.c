#include <stddef.h>
#include <stdint.h>

#include <kernel/desc.h>
#include <sys/io.h>

static const uint8_t PIC_ACK = 0x20;

static inline uint8_t pic_command(pic_port_t p) {
  return p;
}

static inline uint8_t pic_data(pic_port_t p) {
  return p + 1;
}

void pic_ack(size_t irq) {
  outb(pic_command(PIC_1), PIC_ACK);
  if (irq >= 8) {
    outb(pic_command(PIC_2), PIC_ACK);
  }
}

void irq_handler(irq_state_t* s) {
  pic_ack(s->interrupt);
}
