#include <stddef.h>

enum fb_color {
  FB_COLOR_BLACK = 0,
  FB_COLOR_BLUE = 1,
  FB_COLOR_GREEN = 2,
  FB_COLOR_CYAN = 3,
  FB_COLOR_RED = 4,
  FB_COLOR_MAGENTA = 5,
  FB_COLOR_BROWN = 6,
  FB_COLOR_LIGHT_GREY = 7,
  FB_COLOR_DARK_GREY = 8,
  FB_COLOR_LIGHT_BLUE = 9,
  FB_COLOR_LIGHT_GREEN = 10,
  FB_COLOR_LIGHT_CYAN = 11,
  FB_COLOR_LIGHT_RED = 12,
  FB_COLOR_LIGHT_MAGENTA = 13,
  FB_COLOR_LIGHT_BROWN = 14,
  FB_COLOR_WHITE = 15,
};

extern const size_t FB_WIDTH;
extern const size_t FB_HEIGHT;

extern void fb_writestring(const char* s);
extern void fb_fg(enum fb_color c);
extern void fb_bg(enum fb_color c);
extern void fb_init();
