#ifndef BEANS_KERNEL_MULTIBOOT2_H_
#define BEANS_KERNEL_MULTIBOOT2_H_

#include <stdint.h>

const uint32_t MB2_BOOTLOADER_MAGIC = 0x36d76289;
const uint32_t MB2_TAG_TYPE_END = 0;
const uint32_t MB2_TAG_TYPE_MEM_INFO = 4;
const uint32_t MB2_TAG_TYPE_MMAP = 6;

struct mb2_prologue {
  uint32_t size;
  uint32_t unused;
} __attribute__((packed));

struct mb2_tag {
  uint32_t type;
  uint32_t size;
} __attribute__((packed));

#endif
