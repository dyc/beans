#include <stddef.h>
#include <stdint.h>

#include "elf.h"
#include "io.h"
#include "mem.h"
#include "multiboot.h"
#include "util.h"

#define ERROR_HANG error(__LINE__);

static struct multiboot_info boot_info = {0};

static void kmain() {
  serial_write("                                 \n");
  serial_write("  ________________               \n");
  serial_write(" /  ____ < ow >   \\             \n");
  serial_write(" |                |              \n");
  serial_write(" | ----           |              \n");
  serial_write(" |                |              \n");
  serial_write(" | \\ ^__^         |             \n");
  serial_write(" |                |              \n");
  serial_write(" | \\ (oo)\\_______ |            \n");
  serial_write(" |                |              \n");
  serial_write(" | (__)\\ )\\/\\     |           \n");
  serial_write(" |                |              \n");
  serial_write(" | ||----w |      |              \n");
  serial_write(" |                |              \n");
  serial_write(" \\ || ||          /             \n");
  serial_write("  ----------------               \n");
  serial_write("         \\   ^__^               \n");
  serial_write("          \\  (oo)\\_______      \n");
  serial_write("             (__)\\       )\\/\\ \n");
  serial_write("                 ||----w |       \n");
  serial_write("                 ||     ||       \n");
  serial_write(" =============================== \n");
  while (1)
    ;
}

static void error(size_t line_num) {
  char buf[16] = {0};
  sprintf(buf, "err: %d", line_num);
  serial_write(buf);
  while (1)
    ;
}

// todo: ...is there another way to do this?
void loadk(size_t smaps, struct smap_entry *smap, uint32_t *kernel,
           uint32_t *initrd) __attribute__((section(".text.loadk")));
void loadk(size_t smaps, struct smap_entry *smap, uint32_t *kernel,
           uint32_t *initrd) {
  (void)initrd;
  serial_enable();

  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (ELF_IDENT_MAGIC0 != kernel_elf->ident[0] ||
      ELF_IDENT_MAGIC1 != kernel_elf->ident[1] ||
      ELF_IDENT_MAGIC2 != kernel_elf->ident[2] ||
      ELF_IDENT_MAGIC3 != kernel_elf->ident[3]) {
    ERROR_HANG;
  }
  if (ELF_IDENT_32BIT != kernel_elf->ident[4]) {
    ERROR_HANG;
  }
  if (ELF_IDENT_L_ENDIAN != kernel_elf->ident[5]) {
    ERROR_HANG;
  }
  if (ELF_TYPE_EXE != kernel_elf->type) {
    ERROR_HANG;
  }
  if (ELF_ISA_X86 != kernel_elf->isa) {
    ERROR_HANG;
  }

  for (size_t i = 0; i < smaps; ++i) {
    struct smap_entry s = smap[i];

    char buf[64] = {0};
    sprintf(buf, "smap[%d] base: %lld length: %lld type: %d\n", i, s.base,
            s.length, s.type);
    serial_write(buf);

    if (0 == s.base) {
      boot_info.mem_lower = s.length;
    }

    if (0x100000 <= s.base && SMAP_TYPE_RAM == s.type) {
      boot_info.mem_upper += s.length;
    }
  }
  char buf[64] = {0};
  sprintf(buf, "[boot_info.mem] lower: %d upper:\n", boot_info.mem_lower,
          boot_info.mem_upper);
  serial_write(buf);

  kmain();
}
