#include <kernel/desc.h>
#include <kernel/fs.h>
#include <kernel/libc.h>
#include <kernel/mem.h>
#include <kernel/multiboot2.h>
#include <kernel/printf.h>
#include <kernel/serial.h>
#include <kernel/vga.h>
#include <sys/device.h>
#include <sys/kbd.h>

#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    int n = sprintf(buf, "[%s:%d] ", __func__, __LINE__);                      \
    sprintf(&buf[n], fmt, ##__VA_ARGS__);                                      \
    serial_write(SERIAL_PORT_COM1, buf);                                       \
  };

static char buf[256] = {0};
static struct mb2_module *initrd_module = NULL;
static struct mb2_mem_info *mem_info = NULL;
static struct mb2_mmap *mmap = NULL;

struct fnode *mount_initrd(uintptr_t initrd);

void timer_heartbeat(unsigned long t) {
  // heartbeat every 10 seconds
  if (t % 1000 == 0) {
    sprintf(buf, "%ds since boot\n", t / 100);
    serial_write(SERIAL_PORT_COM1, buf);
  }
}

static inline struct mb2_tag *next_tag(struct mb2_tag *tag) {
  return (struct mb2_tag *)(((uint8_t *)tag + tag->size) +
                            (uintptr_t)((uint8_t *)tag + tag->size) % 8);
}

void kmain(struct mb2_prologue *mb2, uint32_t mb2_magic) {
  serial_enable(SERIAL_PORT_COM1);
  PRINTF("serial enabled\n")
  PRINTF("mb2 %x (mb2 & 0x7 = %d)\n", (uint32_t)mb2, ((uint32_t)mb2 & 0x7))

  gdt_install();
  PRINTF("gdt ready\n")
  idt_install();
  PRINTF("idt ready\n")

  if (MB2_BOOTLOADER_MAGIC != mb2_magic) {
    return;
  }

  if ((uint32_t)mb2 & 0x7) {
    return;
  }

  // collect requisite mb2 tags
  for (struct mb2_tag *tag =
           (struct mb2_tag *)((uint8_t *)mb2 + sizeof(struct mb2_prologue));
       MB2_TAG_TYPE_END != tag->type; tag = next_tag(tag)) {
    PRINTF("tag type %d\n", tag->type)
    switch (tag->type) {
    case MB2_TAG_TYPE_MODULE: {
      struct mb2_module *module = (struct mb2_module *)tag;
      PRINTF("found module with string %s\n", module->string)
      if (!strcmp(module->string, "initrd")) {
        initrd_module = module;
      }
      break;
    }
    case MB2_TAG_TYPE_MEM_INFO: {
      mem_info = (struct mb2_mem_info *)tag;
      break;
    }
    case MB2_TAG_TYPE_MMAP: {
      mmap = (struct mb2_mmap *)tag;
      break;
    }
    }
  }
  PRINTF("collected mb2 info\n")

  if (NULL == mem_info) {
    PRINTF("didn't find mem_info tag!\n")
    return;
  }
  paging_init(mem_info->mem_lower + mem_info->mem_upper);

  if (NULL == mmap) {
    PRINTF("didn't find mmap tag!\n")
    return;
  }
  for (struct mb2_mmap_entry *entry = mmap->entries;
       (uintptr_t)entry < (uintptr_t)((uint8_t *)mmap + mmap->tag.size);
       ++entry) {
    PRINTF("mmap @ %x base %lx size %lx type %d\n", (uint32_t)entry,
           (long)entry->base, (long)entry->size, entry->type)
    if (MB2_MMAP_AVAILABLE == entry->type) {
      for (uintptr_t p = entry->base; p < entry->base + entry->size;
           p += PAGE_SIZE_BYTES) {
        paging_mark_avail(p);
      }
    }
  }
  PRINTF("paging ready\n")

  heap_init();
  PRINTF("heap ready\n")

  if (NULL == initrd_module) {
    PRINTF("didn't find initrd_module!\n")
    return;
  }
  PRINTF("mounting initrd located at %x...\n", initrd_module->start)
  mount_initrd(initrd_module->start);

  irq_install();
  PRINTF("irq ready\n")

  pit_install();
  pit_set_freq_hz(100);
  pit_set_timer_cb(timer_heartbeat);
  PRINTF("pit ready\n")

  vga_init();
  vga_fg(VGA_COLOR_BLACK);
  vga_bg(VGA_COLOR_WHITE);
  PRINTF("vga ready\n")

  keyboard_install();
  PRINTF("kbd ready\n")

  memset(buf, 0, sizeof(buf) / sizeof(buf[0]));
  while (1) {
    for (size_t i = 0; i < KEYBOARD_CURSOR; ++i) {
      buf[i] = scancode(KEYBOARD_BUFFER[i]);
    }
    KEYBOARD_CURSOR = 0;
    vga_write(buf);
  }
}
