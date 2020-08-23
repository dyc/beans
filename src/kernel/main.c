#include <kernel/fb.h>

void kmain(void) {
  fb_init();

  for (size_t i = 0; i < FB_HEIGHT; i++) {
    fb_writestring("hello\n");
    fb_writestring("world\n");
  }
}
