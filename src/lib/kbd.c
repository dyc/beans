#include <stdbool.h>
#include <stdint.h>

#include <sys/kbd.h>

static const uint8_t KEY_RELEASE_MASK = 0x80;
static unsigned char KMAP_0[128] = {
    0,   27,   '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,
};
static unsigned char KMAP_1[128] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,
};

static bool shift_pressed = false;

char scancode(uint8_t sc) {
  if (sc & KEY_RELEASE_MASK) {
    sc ^= KEY_RELEASE_MASK;
    switch (sc) {
    // shift
    case 0x2A:
    case 0x36:
      shift_pressed = !shift_pressed;
      break;
    default:
      break;
    }
    return 0;
  } else {
    if (KMAP_0[(int)sc] == 0) {
      switch (sc) {
      // shift
      case 0x2A:
      case 0x36:
        shift_pressed = !shift_pressed;
        break;
      default:
        break;
      }
      return 0;
    }
    return shift_pressed ? KMAP_1[sc] : KMAP_0[sc];
  }
}
