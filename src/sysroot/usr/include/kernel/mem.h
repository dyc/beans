#ifndef BEANS_KERNEL_MEM_H_
#define BEANS_KERNEL_MEM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern const uint32_t PAGE_SIZE_BYTES;
extern char _ld_kernel_phys_end;

struct pde {
  unsigned int present : 1;
  unsigned int read_write : 1;
  unsigned int user_super : 1;
  unsigned int write_through : 1;
  unsigned int cache_disabled : 1;
  unsigned int accessed : 1;
  unsigned int page_size : 1;
  unsigned int unused : 1;
  unsigned int table : 20;
} __attribute__((packed));

struct pte {
  unsigned int present : 1;
  unsigned int read_write : 1;
  unsigned int user_super : 1;
  unsigned int write_through : 1;
  unsigned int cache_disabled : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int global : 1;
  unsigned int frame : 20;
} __attribute__((packed));

extern void paging_init(uintptr_t start, size_t size);
extern void pmap(uintptr_t vaddr, uintptr_t paddr, bool writable, bool user);
extern void punmap(uintptr_t vaddr);

extern void heap_init();

#endif
