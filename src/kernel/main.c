#include <kernel/desc.h>
#include <kernel/serial.h>
#include <sys/device.h>

void kmain(void) {
  gdt_install();
  idt_install();
  irq_install();

  serial_enable(SERIAL_PORT_COM1);
  serial_write(SERIAL_PORT_COM1, "gdt, idt, irq ready\n");

  keyboard_install();
  serial_write(SERIAL_PORT_COM1, "kbd ready\n");

  for(;;) {
    asm("hlt");
  }
}
