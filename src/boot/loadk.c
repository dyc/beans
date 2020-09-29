#include <stddef.h>
#include <stdint.h>

#include "elf.h"
#include "io.h"
#include "mem.h"
#include "util.h"

#define ERROR_HANG error(__LINE__);

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
  char buf[8];
  sprintf(buf, "err: %d\0", line_num);
  serial_write(buf);
  while (1)
    ;
}

// todo: ...is there another way to do this?
void loadk(size_t mmap_entries, struct mmap_entry *mmap, uint32_t *kernel,
           uint32_t *initrd) __attribute__((section(".text.loadk")));
void loadk(size_t mmap_entries, struct mmap_entry *mmap, uint32_t *kernel,
           uint32_t *initrd) {
  (void)mmap_entries;
  (void)mmap;
  (void)initrd;

  serial_enable();

  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (kernel_elf->ident[0] != ELF_IDENT_MAGIC0 ||
      kernel_elf->ident[1] != ELF_IDENT_MAGIC1 ||
      kernel_elf->ident[2] != ELF_IDENT_MAGIC2 ||
      kernel_elf->ident[3] != ELF_IDENT_MAGIC3) {
    ERROR_HANG;
  }
  if (kernel_elf->ident[4] != ELF_IDENT_32BIT) {
    ERROR_HANG;
  }
  if (kernel_elf->ident[5] != ELF_IDENT_L_ENDIAN) {
    ERROR_HANG;
  }
  if (kernel_elf->type != ELF_TYPE_EXE) {
    ERROR_HANG;
  }
  if (kernel_elf->isa != ELF_ISA_X86) {
    ERROR_HANG;
  }

  kmain();
}
