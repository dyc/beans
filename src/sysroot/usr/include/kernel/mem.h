#ifndef BEANS_KERNEL_MEM_H_
#define BEANS_KERNEL_MEM_H_

#include <stddef.h>
#include <stdint.h>

extern const uint32_t PAGE_SIZE_BYTES;

struct pte {
  unsigned int present : 1;
  unsigned int read_write : 1;
  unsigned int user_super : 1;
  unsigned int write_through : 1;
  unsigned int cache_disabled : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int global : 1;
} __attribute__((packed));

extern void paging_init(uintptr_t start, size_t size);
extern void pmap(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);
extern void punmap(uintptr_t vaddr);
// todo: delete this debugging function
extern uint32_t get_num_pages();

extern void heap_init();

#endif
