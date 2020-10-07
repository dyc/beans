// #include <kernel/desc.h>
#include <kernel/libc.h>
#include <kernel/multiboot2.h>
#include <kernel/printf.h>
#include <kernel/serial.h>
// #include <kernel/vga.h>
// #include <sys/device.h>
// #include <sys/kbd.h>

#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    int n = sprintf(buf, "[%s:%d] ", __func__, __LINE__);                      \
    sprintf(&buf[n], fmt, ##__VA_ARGS__);                                      \
    serial_write(SERIAL_PORT_COM1, buf);                                       \
  };

static char buf[256] = {0};
// void timer_heartbeat(unsigned long t) {
//   // heartbeat every 10 seconds
//   if (t % 1000 == 0) {
//     sprintf(buf, "%ds since boot\n", t / 100);
//     serial_write(SERIAL_PORT_COM1, buf);
//   }
// }

void kmain(struct mb2_prologue *mb2, uint32_t mb2_magic) {
  serial_enable(SERIAL_PORT_COM1);
  PRINTF("serial enabled\n");
  PRINTF("mb2 %x (mb2 & 0x7 = %d)\n", (uint32_t)mb2, ((uint32_t)mb2 & 0x7));
  PRINTF("return addr %x\n", __builtin_return_address(0));

  if (MB2_BOOTLOADER_MAGIC != mb2_magic) {
    return;
  }

  if ((uint32_t)mb2 & 0x7) {
    return;
  }

  for (struct mb2_tag *tag = (struct mb2_tag *)(((uint8_t *)mb2) + 8);
       MB2_TAG_TYPE_END != tag->type;
       tag = (struct mb2_tag *)(((uint8_t *)tag) + tag->size)) {
    PRINTF("tag type %d\n", tag->type);
  }

  // gdt_install();
  // idt_install();
  // irq_install();
  // serial_write(SERIAL_PORT_COM1, "gdt, idt, irq ready\n");

  // pit_install();
  // pit_set_freq_hz(100);
  // pit_set_timer_cb(timer_heartbeat);
  // serial_write(SERIAL_PORT_COM1, "pit ready\n");

  // keyboard_install();
  // serial_write(SERIAL_PORT_COM1, "kbd ready\n");

  // if (mb_info->flags & MULTIBOOT_INFO_MODS) {
  //   sprintf(buf, "found %d modules\n", mb_info->mods_count);
  //   serial_write(SERIAL_PORT_COM1, buf);
  //   for (size_t i = 0; i < mb_info->mods_count; ++i) {
  //     sprintf(buf, "mod @ %d\n", mb_info->mods_addr);
  //     serial_write(SERIAL_PORT_COM1, buf);
  //     struct multiboot_module *mod =
  //         (struct multiboot_module *)mb_info->mods_addr;
  //     ((void (*)(void))mod->mod_start)();
  //   }
  // }
  // serial_write(SERIAL_PORT_COM1, "finished loading modules\n");

  // size_t kbdc = 0;
  // char kbdbuf[8] = {0};
  // while (1) {
  //   if (kbdc == KEYBOARD_CURSOR) {
  //     asm volatile("nop");
  //     continue;
  //   }

  //   size_t written = 0;
  //   bool wrap = KEYBOARD_CURSOR < kbdc;
  //   // write to end of buffer or to cursor, if wrap around
  //   size_t n = wrap ? sizeof(KEYBOARD_BUFFER) : KEYBOARD_CURSOR;
  //   for (size_t i = kbdc; i < n; ++i) {
  //     kbdbuf[i - kbdc] = scancode(KEYBOARD_BUFFER[i]);
  //   }
  //   written = n - kbdc;
  //   // handle wrap around
  //   if (wrap) {
  //     for (size_t i = 0; i < KEYBOARD_CURSOR; ++i) {
  //       kbdbuf[written + i] = scancode(KEYBOARD_BUFFER[i]);
  //     }
  //     written += KEYBOARD_CURSOR;
  //   }
  //   kbdbuf[written] = 0;
  //   // todo: change this back to vga_write(kbdbuf);
  //   serial_write(SERIAL_PORT_COM1, kbdbuf);
  //   // if we get interrupt here we may skip over some keys, that's ok
  //   kbdc = KEYBOARD_CURSOR;
  // }
}
