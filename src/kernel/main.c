#include <kernel/desc.h>
#include <kernel/serial.h>
#include <kernel/vga.h>
#include <sys/device.h>
#include <sys/kbd.h>

void kmain(void) {
  gdt_install();
  idt_install();
  irq_install();

  serial_enable(SERIAL_PORT_COM1);
  serial_write(SERIAL_PORT_COM1, "gdt, idt, irq ready\n");

  keyboard_install();
  serial_write(SERIAL_PORT_COM1, "kbd ready\n");

  vga_init();
  serial_write(SERIAL_PORT_COM1, "vga ready\n");

  size_t kbdc = 0;
  char kbdbuf[256];

  for(;;) {
    if (kbdc == KEYBOARD_CURSOR) {
      asm("hlt");
      continue;
    }

    size_t written = 0;
    // write to end of buffer or to cursor, if wrap around
    size_t n = kbdc > KEYBOARD_CURSOR ? sizeof(KEYBOARD_BUFFER): KEYBOARD_CURSOR;
    for (size_t i = kbdc; i < n; ++i) {
      kbdbuf[i - kbdc] = kbd_scancode(KEYBOARD_BUFFER[i]);
    }
    written = n - kbdc;
    // handle wrap around
    if (n > KEYBOARD_CURSOR) {
      for (size_t i = 0; i < KEYBOARD_CURSOR; ++i) {
        kbdbuf[written + i] = kbd_scancode(KEYBOARD_BUFFER[i]);
      }
    }
    kbdbuf[written + 1] = 0;
    vga_write(kbdbuf);
    // if we get interrupt here we may skip over some keys, that's ok
    kbdc = KEYBOARD_CURSOR;
  }
}
