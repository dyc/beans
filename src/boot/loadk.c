#include <stddef.h>
#include <stdint.h>

#include "elf.h"
#include "io.h"
#include "mem.h"
#include "multiboot.h"
#include "util.h"

#define ERROR_HANG error(__LINE__);
#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    sprintf(buf, fmt, ##__VA_ARGS__);                                          \
    serial_write(buf);                                                         \
  };

static struct multiboot_info boot_info = {0};
static char buf[64] = {0};
static uint32_t kentry = 0;

static void error(size_t line_num) {
  PRINTF("err: %d", line_num);
  while (1)
    ;
}

static void woohoo() {
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
  serial_write("                                 \n");
  uint32_t ret = (uint32_t)__builtin_return_address(0);
  PRINTF("[woohoo] mb header: %x\n", (uint32_t)&boot_info);
  PRINTF("[woohoo] return addr: %x\n", ret);

  // todo: fix return addr
  asm volatile("mov %0, %%eax\n"
               "mov %1, %%ebx\n"
               "pushl %%eax\n"
               "pushl %%ebx\n"
               "pushl %2\n"
               "jmp *%3\n" ::"i"(MULTIBOOT_BOOTLOADER_MAGIC),
               "p"((uint32_t)&boot_info), "p"(ret), "g"(kentry));
}

// todo: ...is there another way to do this?
void loadk(size_t smaps, struct smap_entry *smap, uint32_t *kernel,
           uint32_t *initrd) __attribute__((section(".text.loadk")));
void loadk(size_t smaps, struct smap_entry *smap, uint32_t *kernel,
           uint32_t *initrd) {
  (void)initrd;
  serial_enable();

  for (size_t i = 0; i < smaps; ++i) {
    struct smap_entry s = smap[i];
    PRINTF("smap[%d] base: %lx length: %lx type: %d\n", i, (long)s.base,
           (long)s.length, s.type);
    if (0 == s.base) {
      boot_info.mem_lower = s.length;
    }
    if (0x100000 <= s.base && SMAP_TYPE_RAM == s.type) {
      boot_info.mem_upper += s.length;
    }
  }
  PRINTF("[boot_info.mem] lower: %x upper: %x\n", boot_info.mem_lower,
         boot_info.mem_upper);

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

  kentry = kernel_elf->entry;
  PRINTF("kentry: %x\n", kentry)

  uint8_t *pheader_base = ((uint8_t *)kernel) + kernel_elf->ph_offset_bytes;
  PRINTF("reading %d pheaders starting at %x\n", kernel_elf->ph_ents,
         pheader_base);
  for (size_t i = 0; i < kernel_elf->ph_ents; ++i) {
    struct elf_pheader *pheader = (struct elf_pheader *)pheader_base +
                                  (i * kernel_elf->ph_ent_size_bytes);
    PRINTF("pheaders[%d] type: %d vaddr: %x memsize: %x\n", i, pheader->type,
           pheader->virt_addr, pheader->memsize_bytes);
    if (ELF_PHTYPE_LOAD == pheader->type) {
      PRINTF("loading data seg (%d bytes) from elf offset %x to vaddr %x\n",
             pheader->filesize_bytes, pheader->offset, pheader->virt_addr);

      memcpy((void *)pheader->virt_addr, (void *)kernel + pheader->offset,
             pheader->memsize_bytes);
      if (pheader->filesize_bytes < pheader->memsize_bytes) {
        memset((void *)pheader->virt_addr + pheader->filesize_bytes, 0,
               pheader->memsize_bytes - pheader->filesize_bytes);
      }
    }
  }

  woohoo();
  ERROR_HANG
}
