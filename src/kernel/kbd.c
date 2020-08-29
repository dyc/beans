#include <kernel/desc.h>
#include <kernel/vga.h>

#include <sys/device.h>
#include <sys/io.h>

const uint8_t KEYBOARD_IRQ = 0x01;
static const uint8_t KEYBOARD_PORT = 0x60;
static const uint8_t KEYBOARD_PENDING = 0x64;
char buf[2] = {};

int keyboard_handler(irq_state_t* s) {
  (void)(s);
  if (inb(KEYBOARD_PENDING) & 0x01) {
    buf[0] = inb(KEYBOARD_PORT);
    vga_write(buf);
  }
  pic_ack(KEYBOARD_IRQ);
  return 1;
}

void keyboard_install() {
  vga_init();
  irq_install_handler(KEYBOARD_IRQ, keyboard_handler);
}

