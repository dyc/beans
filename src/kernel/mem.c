#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

uintptr_t free_ptr;
const uint32_t PAGE_SIZE_BYTES = 0x1000;

void paging_init(size_t s) {
  (void)s;
  return;
}

void paging_mark_avail(uintptr_t addr) {
  (void)addr;
  return;
}

void heap_init() {}
