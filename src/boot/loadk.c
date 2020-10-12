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

// 8 byte aligned home for mb2 info
static struct mb2_prologue *prologue = (struct mb2_prologue *)0x200000;
static char buf[256] = {0};

static void error() {
  PRINTF(":-(");
  while (1)
    ;
}

static void woohoo(uint32_t kentry) {
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
  PRINTF("mb2 %x\n", (uint32_t)prologue);

  asm volatile("mov %0, %%eax\n"
               "mov %1, %%ebx\n"
               "pushl %%eax\n"
               "pushl %%ebx\n"
               "jmp *%2\n" ::"i"(MB2_BOOTLOADER_MAGIC),
               "g"((uint32_t)prologue), "g"(kentry));
}

__attribute__((section(".text.loadk"))) void loadk(size_t smaps,
                                                   struct smap_entry *smap,
                                                   uint32_t *kernel,
                                                   uint32_t *initrd) {
  serial_enable();

  // ---- load kernel --------
  struct elf_header *kernel_elf = (struct elf_header *)kernel;
  if (ELF_IDENT_MAGIC0 != kernel_elf->ident[0] ||
      ELF_IDENT_MAGIC1 != kernel_elf->ident[1] ||
      ELF_IDENT_MAGIC2 != kernel_elf->ident[2] ||
      ELF_IDENT_MAGIC3 != kernel_elf->ident[3]) {
    error();
  }
  if (ELF_IDENT_32BIT != kernel_elf->ident[4]) {
    error();
  }
  if (ELF_IDENT_L_ENDIAN != kernel_elf->ident[5]) {
    error();
  }
  if (ELF_TYPE_EXE != kernel_elf->type) {
    error();
  }
  if (ELF_ISA_X86 != kernel_elf->isa) {
    error();
  }

  const uint32_t kentry = kernel_elf->entry;
  PRINTF("kentry %x\n", kentry)

  uint8_t *pheader_base = ((uint8_t *)kernel) + kernel_elf->ph_offset_bytes;
  PRINTF("reading %d pheaders starting at %x\n", kernel_elf->ph_ents,
         (uint32_t)pheader_base);
  for (size_t i = 0; i < kernel_elf->ph_ents; ++i) {
    struct elf_pheader *pheader = (struct elf_pheader *)pheader_base +
                                  (i * kernel_elf->ph_ent_size_bytes);
    PRINTF("pheaders[%ld] type %d vaddr %lx memsize %x\n", i, pheader->type,
           (long)pheader->virt_addr, pheader->memsize_bytes);
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

  uint32_t mb2_end =
      (uint32_t)((uint8_t *)prologue + sizeof(struct mb2_prologue));
  // ---- mb2 meminfo --------
  struct mb2_mem_info *mem_info = (struct mb2_mem_info *)mb2_end;
  mem_info->tag.type = MB2_TAG_TYPE_MEM_INFO;
  mem_info->tag.size = sizeof(struct mb2_mem_info);
  mb2_end += (uint32_t)((uint8_t *)mb2_end + sizeof(struct mb2_mem_info));

  // ---- mb2 mmap --------
  struct mb2_mmap *mmap = (struct mb2_mmap *)mb2_end;
  mmap->tag.type = MB2_TAG_TYPE_MMAP;
  mmap->tag.size =
      sizeof(struct mb2_mmap) + sizeof(struct mb2_mmap_entry) * smaps;
  mmap->entry_size = sizeof(struct mb2_mmap_entry);
  mmap->entries =
      (struct mb2_mmap_entry *)((uint8_t *)mb2_end + sizeof(struct mb2_mmap));
  struct mb2_mmap_entry *mmap_entry = (struct mb2_mmap_entry *)mmap->entries;
  for (size_t i = 0; i < smaps; ++i) {
    struct smap_entry s = smap[i];
    PRINTF("smap[%ld] base %lx size %lx type %d\n", i, (long)s.base,
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
    PRINTF("mmap[%x] base %lx size %lx type %d\n", (uint32_t)mmap_entry,
           (long)mmap_entry->base, (long)mmap_entry->size, mmap_entry->type);
    ++mmap_entry;
  }
  PRINTF("boot_info.mem: lower %x upper %x\n", mem_info->mem_lower,
         mem_info->mem_upper);
  mb2_end = (uint32_t)(++mmap_entry);

  // ---- mb2 modules --------
  struct mb2_module *initrd_mod = (struct mb2_module *)mb2_end;
  initrd_mod->tag.type = MB2_TAG_TYPE_MODULE;
  initrd_mod->start = (uint32_t)initrd;
  // kernel takes initrd as a linkedlist, so don't need to know its size
  initrd_mod->end = 0;
  sprintf(initrd_mod->string, "initrd");
  // include initrd_mod->string and its null byte
  initrd_mod->tag.size =
      sizeof(struct mb2_module) + strlen(initrd_mod->string) + 1;
  mb2_end = (uint32_t)((uint8_t *)mb2_end + initrd_mod->tag.size);

  // ---- mb2 sentinel --------
  struct mb2_tag *sentinel = (struct mb2_tag *)mb2_end;
  sentinel->type = MB2_TAG_TYPE_END;
  sentinel->size = sizeof(struct mb2_tag);
  mb2_end = (uint32_t)((uint8_t *)mb2_end + sizeof(struct mb2_tag));

  prologue->size = mb2_end - (uint32_t)prologue;
  PRINTF("final mb2 size %d bytes\n", prologue->size);

  woohoo(kentry);
}
