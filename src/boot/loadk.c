#include <stddef.h>
#include <stdint.h>

#include "elf.h"
#include "io.h"
#include "mem.h"
#include "multiboot2.h"
#include "util.h"

#define ERROR_HANG error(__LINE__);
#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    sprintf(buf, fmt, ##__VA_ARGS__);                                          \
    serial_write(buf);                                                         \
  };

static const uint32_t KERNEL_START_PHYS = 0x100000;
static char buf[64] = {0};
static uint32_t kentry = 0;
static uintptr_t mb2_addr = 0;

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
  PRINTF("[woohoo] mb2: %x\n", (uint32_t)mb2_addr);
  PRINTF("[woohoo] return addr: %x\n", ret);

  // todo: fix return addr
  asm volatile("mov %0, %%eax\n"
               "mov %1, %%ebx\n"
               "pushl %%eax\n"
               "pushl %%ebx\n"
               "pushl %2\n"
               "jmp *%3\n" ::"i"(MB2_BOOTLOADER_MAGIC),
               "p"((uint32_t)mb2_addr), "p"(ret), "g"(kentry));
}

__attribute__((section(".text.loadk"))) void loadk(size_t smaps,
                                                   struct smap_entry *smap,
                                                   uint32_t *kernel,
                                                   uint32_t *initrd) {
  (void)initrd;
  serial_enable();

  size_t mmap_size = sizeof(struct mb2_mmap_entry) * smaps;
  PRINTF("allocating %d bytes for %d mb2 mmap entries at %x\n", mmap_size,
         smaps, KERNEL_START_PHYS);
  memset((void *)KERNEL_START_PHYS, 0, mmap_size);

  mb2_addr = KERNEL_START_PHYS + mmap_size;
  PRINTF("starting mb2 after mmap entries at %x\n", mb2_addr);
  uintptr_t mb2_end = mb2_addr;

  struct mb2_prologue *prologue = (struct mb2_prologue *)mb2_addr;
  mb2_end = (uintptr_t)((uint8_t *)mb2_end + sizeof(prologue));

  struct mb2_mem_info *mem_info = (struct mb2_mem_info *)mb2_end;
  mem_info->tag.type = MB2_TAG_TYPE_MEM_INFO;
  mem_info->tag.size = sizeof(mem_info);
  mb2_end = (uintptr_t)((uint8_t *)mb2_end + sizeof(mem_info));

  struct mb2_mmap_entry *mmap_entry =
      (struct mb2_mmap_entry *)KERNEL_START_PHYS;
  struct mb2_mmap *mmap = (struct mb2_mmap *)mb2_end;
  mmap->tag.type = MB2_TAG_TYPE_MMAP;
  mmap->tag.size = sizeof(mmap);
  mmap->entry_size = sizeof(mmap_entry);
  mmap->entries = mmap_entry;
  mb2_end = (uintptr_t)((uint8_t *)mb2_end + sizeof(mmap));
  for (size_t i = 0; i < smaps; ++i) {
    struct smap_entry s = smap[i];
    PRINTF("smap[%d] base: %lx size: %lx type: %d\n", i, (long)s.base,
           (long)s.size, s.type);
    if (0 == s.base) {
      mem_info->mem_lower = s.size;
    }
    if (0x100000 <= s.base && SMAP_TYPE_RAM == s.type) {
      mem_info->mem_upper += s.size;
    }

    mmap_entry->base = s.base;
    mmap_entry->size = s.size;
    mmap_entry->type = s.type;
    PRINTF("mmap[%x] base: %lx size: %lx type: %d\n", mmap_entry,
           (long)mmap_entry->base, (long)mmap_entry->size, mmap_entry->type);
    ++mmap_entry;
  }
  PRINTF("[boot_info.mem] lower: %x upper: %x\n", mem_info->mem_lower,
         mem_info->mem_upper);

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

  prologue->size = (uint32_t)mb2_end - mb2_addr;
  woohoo();
  ERROR_HANG
}
