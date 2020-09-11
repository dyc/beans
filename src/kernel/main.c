#include <kernel/desc.h>
#include <kernel/multiboot.h>
#include <kernel/printf.h>
#include <kernel/serial.h>
// #include <kernel/vga.h>
#include <sys/device.h>
#include <sys/kbd.h>

static char scratchbuf[26];
void timer_heartbeat(unsigned long t) {
  // heartbeat every 10 seconds
  if (t % 1000 == 0) {
    sprintf(scratchbuf, "%ds since boot\n", t / 100);
    serial_write(SERIAL_PORT_COM1, scratchbuf);
  }
}

void kmain(struct multiboot_info *mb_info, uint32_t mb_magic) {
  serial_enable(SERIAL_PORT_COM1);
  serial_write(SERIAL_PORT_COM1, "serial enabled\n");

  gdt_install();
  idt_install();
  irq_install();
  serial_write(SERIAL_PORT_COM1, "gdt, idt, irq ready\n");

  pit_install();
  pit_set_freq_hz(100);
  pit_set_timer_cb(timer_heartbeat);
  serial_write(SERIAL_PORT_COM1, "pit ready\n");

  keyboard_install();
  serial_write(SERIAL_PORT_COM1, "kbd ready\n");

  if (MULTIBOOT_BOOTLOADER_MAGIC != mb_magic) {
    sprintf(scratchbuf, "eax: %d\n", mb_magic);
    serial_write(SERIAL_PORT_COM1, "multiboot magic check failed\n");
    serial_write(SERIAL_PORT_COM1, scratchbuf);
    while (1)
      ;
  } else {
    serial_write(SERIAL_PORT_COM1, "multiboot magic seems ok\n");
  }

  if (mb_info->flags & MULTIBOOT_INFO_MODS) {
    sprintf(scratchbuf, "found %d modules\n", mb_info->mods_count);
    serial_write(SERIAL_PORT_COM1, scratchbuf);
    for (size_t i = 0; i < mb_info->mods_count; ++i) {
      sprintf(scratchbuf, "mod @ %d\n", mb_info->mods_addr);
      serial_write(SERIAL_PORT_COM1, scratchbuf);
      struct multiboot_module *mod =
          (struct multiboot_module *)mb_info->mods_addr;
      ((void (*)(void))mod->mod_start)();
    }
  }
  serial_write(SERIAL_PORT_COM1, "finished loading modules\n");

  size_t kbdc = 0;
  char kbdbuf[8];
  while (1) {
    if (kbdc == KEYBOARD_CURSOR) {
      asm("hlt");
      continue;
    }

    size_t written = 0;
    bool wrap = KEYBOARD_CURSOR < kbdc;
    // write to end of buffer or to cursor, if wrap around
    size_t n = wrap ? sizeof(KEYBOARD_BUFFER) : KEYBOARD_CURSOR;
    for (size_t i = kbdc; i < n; ++i) {
      kbdbuf[i - kbdc] = scancode(KEYBOARD_BUFFER[i]);
    }
    written = n - kbdc;
    // handle wrap around
    if (wrap) {
      for (size_t i = 0; i < KEYBOARD_CURSOR; ++i) {
        kbdbuf[written + i] = scancode(KEYBOARD_BUFFER[i]);
      }
      written += KEYBOARD_CURSOR;
    }
    kbdbuf[written] = 0;
    // todo: change this back to vga_write(kbdbuf);
    serial_write(SERIAL_PORT_COM1, kbdbuf);
    // if we get interrupt here we may skip over some keys, that's ok
    kbdc = KEYBOARD_CURSOR;
  }
}
