#ifndef BEANS_BOOT_ELF_H_
#define BEANS_BOOT_ELF_H_

#include <stdint.h>

#define ELF_IDENT_N 16
#define ELF_IDENT_MAGIC0 0x7f
#define ELF_IDENT_MAGIC1 'E'
#define ELF_IDENT_MAGIC2 'L'
#define ELF_IDENT_MAGIC3 'F'
#define ELF_IDENT_32BIT 1
#define ELF_IDENT_L_ENDIAN 1
#define ELF_TYPE_EXE 2
#define ELF_ISA_X86 3

struct elf_header {
  uint8_t ident[ELF_IDENT_N];
  uint16_t type;
  uint16_t isa;
  uint32_t version;
  uint32_t entry;
  uint32_t ph_offset_bytes;
  uint32_t sh_offset_bytes;
  uint32_t flags;
  uint16_t header_size_bytes;
  uint16_t ph_ent_size_bytes;
  uint16_t ph_ents;
  uint16_t sh_ent_size_bytes;
  uint16_t sh_ents;
  uint16_t sh_strtab_idx;
} __attribute__((packed));

#endif
