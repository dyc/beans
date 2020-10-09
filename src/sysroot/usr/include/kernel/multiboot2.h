#ifndef BEANS_KERNEL_MULTIBOOT2_H_
#define BEANS_KERNEL_MULTIBOOT2_H_

#include <stdint.h>

const uint32_t MB2_BOOTLOADER_MAGIC = 0x36d76289;
const uint32_t MB2_TAG_TYPE_END = 0;
const uint32_t MB2_TAG_TYPE_MODULE = 3;
const uint32_t MB2_TAG_TYPE_MEM_INFO = 4;
const uint32_t MB2_TAG_TYPE_MMAP = 6;

const uint32_t MB2_MMAP_AVAILABLE = 1;
const uint32_t MB2_MMAP_RESERVED = 2;
const uint32_t MB2_MMAP_ACPI_RECLAIMABLE = 3;
const uint32_t MB2_MMAP_ACPI_NVS = 4;
const uint32_t MB2_MMAP_BAD = 5;

struct mb2_prologue {
  uint32_t size;
  uint32_t unused;
} __attribute__((packed));

struct mb2_tag {
  uint32_t type;
  uint32_t size;
} __attribute__((packed));

struct mb2_mmap_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t unused;
} __attribute__((packed));

struct mb2_mmap {
  struct mb2_tag tag;
  uint32_t entry_size;
  uint32_t entry_version;
  struct mb2_mmap_entry *entries;
} __attribute__((packed));

struct mb2_mem_info {
  struct mb2_tag tag;
  uint32_t mem_lower;
  uint32_t mem_upper;
} __attribute__((packed));

struct mb2_module {
  struct mb2_tag tag;
  uint32_t start;
  uint32_t end;
  char string[0];
} __attribute__((packed));

#endif
