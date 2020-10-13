#ifndef BEANS_KERNEL_MEM_H_
#define BEANS_KERNEL_MEM_H_

#include <stddef.h>
#include <stdint.h>

extern const uint32_t PAGE_SIZE_BYTES;

extern void paging_init(size_t s);
extern void paging_mark_avail(uintptr_t addr);
extern void heap_init();

#endif