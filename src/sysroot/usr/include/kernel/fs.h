#ifndef BEANS_KERNEL_FS_H_
#define BEANS_KERNEL_FS_H_

#include <stdint.h>

struct fnode {
  char name[64];
  uint32_t size;
} __attribute__((packed));

#endif
