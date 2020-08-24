#include <kernel/serial.h>
#include <kernel/vga.h>

void kmain(void) {
  vga_init();

  for (size_t i = 0; i < VGA_HEIGHT; i++) {
    vga_fg(VGA_COLOR_MAGENTA);
    vga_bg(VGA_COLOR_BROWN);
    vga_writestring("howdy\n");

    vga_fg(VGA_COLOR_CYAN);
    vga_bg(VGA_COLOR_GREEN);
    vga_writestring("howdy\n");
  }
  vga_fg(VGA_COLOR_LIGHT_GREY);
  vga_bg(VGA_COLOR_BLACK);
  vga_writestring("woooorld\nboop");

  serial_enable(SERIAL_PORT_COM1);
}
