#include <kernel/desc.h>

#include <sys/device.h>
#include <sys/io.h>

char KEYBOARD_BUFFER[128];
size_t KEYBOARD_CURSOR = 0;

static const uint8_t KEYBOARD_IRQ = 0x01;
static const uint8_t KEYBOARD_PORT = 0x60;
static const uint8_t KEYBOARD_PENDING = 0x64;

static int keyboard_handler(irq_state_t* s) {
  (void)(s);
  if (inb(KEYBOARD_PENDING) & 0x01) {
    KEYBOARD_BUFFER[KEYBOARD_CURSOR++] = inb(KEYBOARD_PORT);
    if (KEYBOARD_CURSOR >= sizeof(KEYBOARD_BUFFER)) {
      KEYBOARD_CURSOR = 0;
    }
  }
  pic_ack(KEYBOARD_IRQ);
  return 1;
}

void keyboard_install() {
  irq_install_handler(KEYBOARD_IRQ, keyboard_handler);
}
