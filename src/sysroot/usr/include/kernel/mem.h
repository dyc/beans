#ifndef BEANS_KERNEL_MEM_H_
#define BEANS_KERNEL_MEM_H_

#include <stddef.h>
#include <stdint.h>

extern const uint32_t PAGE_SIZE_BYTES;

struct Node {
  struct Node *next;
};

extern void paging_init(uintptr_t start, size_t size);
extern void pmap(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);
extern void punmap(uintptr_t vaddr);
// todo: delete this debugging function
extern uint32_t get_num_pages();

extern void heap_init();

#endif
