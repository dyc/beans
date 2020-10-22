#include <kernel/desc.h>
#include <kernel/macros.h>

#include <sys/device.h>
#include <sys/io.h>

// todo: not this
char KEYBOARD_BUFFER[1] = {0};

static const uint8_t KEYBOARD_IRQ = 0x01;
static const uint8_t KEYBOARD_PORT = 0x60;
static const uint8_t KEYBOARD_PENDING = 0x64;

static int keyboard_handler(struct irq_state *s) {
  IGNORE(s)

  if (inb(KEYBOARD_PENDING) & 0x01) {
    KEYBOARD_BUFFER[0] = inb(KEYBOARD_PORT);
  }
  pic_ack(KEYBOARD_IRQ);
  return 1;
}

void keyboard_install() { irq_install_handler(KEYBOARD_IRQ, keyboard_handler); }
