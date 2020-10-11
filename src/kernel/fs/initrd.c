#include <stddef.h>
#include <stdint.h>

#include <kernel/fs.h>
#include <kernel/libc.h>

struct fnode *mount_initrd(uintptr_t initrd) {
  (void)initrd;
  struct fnode *node = malloc(sizeof(struct fnode));
  memset(node, 0, sizeof(struct fnode));
  node->inode = initrd;
  return NULL;
}
