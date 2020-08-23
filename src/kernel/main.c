#include <kernel/fb.h>

void kmain(void) {
  fb_init();

  for (size_t i = 0; i < FB_HEIGHT; i++) {
    fb_fg(FB_COLOR_MAGENTA);
    fb_bg(FB_COLOR_BROWN);
    fb_writestring("hello\n");

    fb_fg(FB_COLOR_CYAN);
    fb_bg(FB_COLOR_GREEN);
    fb_writestring("world\n");
  }
}
