#ifndef BEANS_BOOT_MEM_H_
#define BEANS_BOOT_MEM_H_

#include <stdint.h>

struct mmap_entry {
  uint64_t base, length;
  uint32_t type, unused;
} __attribute__((packed));

#endif
