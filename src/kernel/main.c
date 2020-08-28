#include <kernel/desc.h>
// #include <kernel/printf.h>
#include <kernel/serial.h>
// #include <kernel/vga.h>
#include <sys/device.h>

void kmain(void) {
  gdt_install();
  idt_install();
  irq_install();
  serial_enable(SERIAL_PORT_COM1);
  keyboard_install();
  asm volatile("sti");
  for(;;) {
    asm("hlt");
  }

  // vga_init();
  // serial_enable(SERIAL_PORT_COM1);


  // char buf[8];
  // for (size_t i = 0; i < VGA_HEIGHT; ++i) {
  //   vga_fg(VGA_COLOR_MAGENTA);
  //   vga_bg(VGA_COLOR_BROWN);
  //   vga_write("howdy\n");

  //   vga_fg(VGA_COLOR_CYAN);
  //   vga_bg(VGA_COLOR_GREEN);
  //   vga_write("howdy\n");

  //   sprintf(buf, "i: %d\n", i);
  //   serial_write(SERIAL_PORT_COM1, buf);
  // }
  // vga_fg(VGA_COLOR_LIGHT_GREY);
  // vga_bg(VGA_COLOR_BLACK);
  // vga_write("woooorld\nboop");
  // serial_write(SERIAL_PORT_COM1, "woooorld\nboop");
}
