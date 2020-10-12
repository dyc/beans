#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

uintptr_t free_ptr;

void paging_init(size_t s) {
  (void)s;
  return;
}

void paging_mark_avail(uintptr_t addr) {
  (void)addr;
  return;
}

void heap_init() {}
