#include <kernel/desc.h>
#include <kernel/serial.h>

#include <sys/device.h>
#include <sys/io.h>

const uint8_t KEYBOARD_IRQ = 0x01;
static const uint8_t KEYBOARD_PORT = 0x60;
static const uint8_t KEYBOARD_PENDING = 0x64;
static char buf[] = {'i', 'n', 't', '\n', 0};

int keyboard_handler(irq_state_t* s) {
  serial_write(SERIAL_PORT_COM1, buf);
  if (inb(KEYBOARD_PENDING) & 0x01) {
    buf[0] = inb(KEYBOARD_PORT);
    // for now, write to serial out
    serial_write(SERIAL_PORT_COM1, buf);
  } else {
    serial_write(SERIAL_PORT_COM1, buf);
  }
  pic_ack(s->interrupt);
  return 1;
}

void keyboard_install() {
  irq_install_handler(KEYBOARD_IRQ, keyboard_handler);
}

