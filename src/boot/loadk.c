#include <stddef.h>
#include <stdint.h>

#include "elf.h"
#include "io.h"
#include "mem.h"
#include "multiboot2.h"
#include "util.h"

#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    int n = sprintf(buf, "[%s:%d] ", __func__, __LINE__);                      \
    sprintf(&buf[n], fmt, ##__VA_ARGS__);                                      \
    serial_write(buf);                                                         \
  };

#define ERROR                                                                  \
  {                                                                            \
    PRINTF(":-(\n")                                                            \
    while (1)                                                                  \
      ;                                                                        \
  };

static char buf[256] = {0};

static inline struct mb2_tag *next_tag(struct mb2_tag *tag) {
  return (struct mb2_tag *)(((uint8_t *)tag + tag->size) +
                            (uintptr_t)((uint8_t *)tag + tag->size) % 8);
}

static void woohoo(uint32_t mb2, uint32_t kentry) {
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

  asm volatile("mov %0, %%eax\n"
               "mov %1, %%ebx\n"
               "pushl %%eax\n"
               "pushl %%ebx\n"
               "jmp *%2\n" ::"i"(MB2_BOOTLOADER_MAGIC),
               "g"(mb2), "g"(kentry));
}

__attribute__((section(".text.loadk"))) void loadk(size_t smaps,
                                                   struct smap_entry *smap,
                                                   uint32_t *kernel,
                                                   uint32_t *initrd) {
  serial_enable();
  PRINTF("%d smaps %lx kernel %lx initrd %lx\n", smaps, (uint32_t) smap, (uint32_t)kernel, (uint32_t)initrd)

  // ---- load kernel --------
  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (ELF_IDENT_MAGIC0 != kernel_elf->ident[0] ||
      ELF_IDENT_MAGIC1 != kernel_elf->ident[1] ||
      ELF_IDENT_MAGIC2 != kernel_elf->ident[2] ||
      ELF_IDENT_MAGIC3 != kernel_elf->ident[3]) {
    ERROR
  }
  if (ELF_IDENT_32BIT != kernel_elf->ident[4]) {
    ERROR
  }
  if (ELF_IDENT_L_ENDIAN != kernel_elf->ident[5]) {
    ERROR
  }
  if (ELF_TYPE_EXE != kernel_elf->type) {
    ERROR
  }
  if (ELF_ISA_X86 != kernel_elf->isa) {
    ERROR
  }

  const uint32_t kentry = kernel_elf->entry;
  PRINTF("kentry %lx\n", kentry)

  uint8_t *pheader_base = ((uint8_t *)kernel) + kernel_elf->ph_offset_bytes;
  PRINTF("reading %d pheaders starting at %x\n", kernel_elf->ph_ents,
         (uint32_t)pheader_base)
  for (size_t i = 0; i < kernel_elf->ph_ents; ++i) {
    struct elf_pheader *pheader = (struct elf_pheader *)pheader_base +
                                  (i * kernel_elf->ph_ent_size_bytes);
    PRINTF("pheaders[%ld] type %d vaddr %lx paddr %lx memsize %x\n", i,
           pheader->type, (long)pheader->virt_addr, (long)pheader->phys_addr,
           pheader->memsize_bytes)
    if (ELF_PHTYPE_LOAD == pheader->type) {
      PRINTF("loading data seg (%d bytes) from elf offset %x to vaddr %x\n",
             pheader->filesize_bytes, pheader->offset, pheader->virt_addr)

      memcpy((void *)pheader->virt_addr, (void *)kernel + pheader->offset,
             pheader->memsize_bytes);
      if (pheader->filesize_bytes < pheader->memsize_bytes) {
        memset((void *)pheader->virt_addr + pheader->filesize_bytes, 0,
               pheader->memsize_bytes - pheader->filesize_bytes);
      }
    }
  }

  // reclaim now relocated kernel elf space for mb2
  memset((void *)kernel, 0, (uintptr_t)initrd - (uintptr_t)kernel);
  // 8 byte aligned mb2 start
  struct mb2_prologue *prologue =
      (struct mb2_prologue *)((uint8_t *)kernel + ((uintptr_t)kernel % 8));
  PRINTF("mb2 prologue %x\n", (uint32_t)prologue)
  // ---- mb2 meminfo --------
  struct mb2_mem_info *mem_info =
      (struct mb2_mem_info *)((uint8_t *)prologue +
                              sizeof(struct mb2_prologue));
  mem_info->tag.type = MB2_TAG_TYPE_MEM_INFO;
  mem_info->tag.size = sizeof(struct mb2_mem_info);

  // ---- mb2 mmap --------
  struct mb2_mmap *mmap = (struct mb2_mmap *)next_tag(&mem_info->tag);
  mmap->tag.type = MB2_TAG_TYPE_MMAP;
  mmap->tag.size =
      sizeof(struct mb2_mmap) + sizeof(struct mb2_mmap_entry) * smaps;
  mmap->entry_size = sizeof(struct mb2_mmap_entry);
  mmap->entries =
      (struct mb2_mmap_entry *)((uint8_t *)mmap + sizeof(struct mb2_mmap));
  struct mb2_mmap_entry *mmap_entry = (struct mb2_mmap_entry *)mmap->entries;
  for (size_t i = 0; i < smaps; ++i) {
    struct smap_entry s = smap[i];
    if (0 == s.base) {
      mem_info->mem_lower = s.size;
    }
    if (0x100000 <= s.base && SMAP_TYPE_RAM == s.type) {
      mem_info->mem_upper += s.size;
    }

    mmap_entry->base = s.base;
    mmap_entry->size = s.size;
    mmap_entry->type = s.type;
    PRINTF("mmap[%ld] base %lx size %lx type %d\n", i, (long)mmap_entry->base,
           (long)mmap_entry->size, mmap_entry->type)
    ++mmap_entry;
  }
  PRINTF("boot_info.mem: lower %x upper %x\n", mem_info->mem_lower,
         mem_info->mem_upper)

  // ---- mb2 modules --------
  struct mb2_module *initrd_mod = (struct mb2_module *)next_tag(&mmap->tag);
  initrd_mod->tag.type = MB2_TAG_TYPE_MODULE;
  initrd_mod->start = (uint32_t)initrd;
  // kernel takes initrd as a linkedlist, so don't need to know its size
  initrd_mod->end = 0;
  sprintf(initrd_mod->string, "initrd");
  // include initrd_mod->string and its null byte
  initrd_mod->tag.size =
      sizeof(struct mb2_module) + strlen(initrd_mod->string) + 1;

  // ---- mb2 sentinel --------
  struct mb2_tag *sentinel = (struct mb2_tag *)next_tag(&initrd_mod->tag);
  sentinel->type = MB2_TAG_TYPE_END;
  sentinel->size = sizeof(struct mb2_tag);

  prologue->size = (uint32_t)next_tag(sentinel) - (uint32_t)prologue;
  PRINTF("final mb2 size %d bytes\n", prologue->size)
  if ((uint32_t)kernel + prologue->size >= (uint32_t)initrd) {
    PRINTF("mb2 is too large and has clobbered initrd :-(\n")
    ERROR
  }

  woohoo((uint32_t)prologue, kentry);
}
