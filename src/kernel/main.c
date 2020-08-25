#include <kernel/serial.h>
#include <kernel/vga.h>

void kmain(void) {
  vga_init();
  serial_enable(SERIAL_PORT_COM1);

  for (size_t i = 0; i < VGA_HEIGHT; ++i) {
    vga_fg(VGA_COLOR_MAGENTA);
    vga_bg(VGA_COLOR_BROWN);
    vga_write("howdy\n");

    vga_fg(VGA_COLOR_CYAN);
    vga_bg(VGA_COLOR_GREEN);
    vga_write("howdy\n");
    serial_write(SERIAL_PORT_COM1, "howdy\n");
  }
  vga_fg(VGA_COLOR_LIGHT_GREY);
  vga_bg(VGA_COLOR_BLACK);
  vga_write("woooorld\nboop");
  serial_write(SERIAL_PORT_COM1, "woooorld\nboop");
}
