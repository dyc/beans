#include <stddef.h>
#include <stdint.h>

#include <kernel/fs.h>

struct fnode *mount_initrd(uintptr_t initrd) {
  (void)initrd;
  // todo: this
  return NULL;
}
