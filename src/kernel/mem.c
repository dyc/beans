#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

uintptr_t free_ptr;
uint32_t num_pages;
const uint32_t PAGE_SIZE_BYTES = 0x1000;

void paging_init(uintptr_t start, size_t size) {
  free_ptr = start;
  num_pages = size / 4;
  return;
}

void paging_mark_avail(uintptr_t addr) {
  (void)addr;
  return;
}

// first-fit heap
// todo: not first-fit heap
void heap_init() {}

uint32_t get_num_pages() { return num_pages; }
