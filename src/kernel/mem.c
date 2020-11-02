#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

// todo: from here
#include <kernel/libc.h>
#include <kernel/printf.h>
#include <kernel/serial.h>

#define PRINTF(fmt, ...)                                                       \
  {                                                                            \
    memset(buf, 0, sizeof(buf) / sizeof(buf[0]));                              \
    int n = sprintf(buf, "[%s:%d] ", __func__, __LINE__);                      \
    sprintf(&buf[n], fmt, ##__VA_ARGS__);                                      \
    serial_write(SERIAL_PORT_COM1, buf);                                       \
  };

static char buf[256] = {0};
// todo: to here

const uint32_t PAGE_SIZE_BYTES = 0x1000;

struct node {
  struct node *next;
};

struct node *free_ptr = NULL;
size_t num_pages;
uintptr_t end;
uintptr_t *pd;

uintptr_t allocate_page() {
  if (free_ptr == NULL || (uintptr_t)free_ptr == end) {
    return 0;
  }

  uintptr_t ret = (uintptr_t)free_ptr;
  free_ptr = free_ptr->next;
  return ret;
}

void free_page(uintptr_t addr) {
  struct node *n = (struct node *)addr;
  n->next = free_ptr;
  free_ptr = n;
}

void invlpg(uintptr_t vaddr) {
  asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

uintptr_t get_cr3() {
  uintptr_t ret;
  asm volatile("mov %%cr3, %%eax" : "=a"(ret));
  return ret;
}

void set_crt3(uintptr_t addr) { asm volatile("mov %%eax, %%cr3" ::"a"(addr)); }

inline size_t pdi(uintptr_t vaddr) { return (vaddr >> 22) & 0x3ff; }

inline size_t pti(uintptr_t vaddr) { return (vaddr >> 12) & 0x3ff; }

uintptr_t get_pte(uintptr_t vaddr, bool create) {
  struct pte *pte = (struct pte *)pd[pdi(vaddr)];
  PRINTF("pte: %lx\n", pte)
  if (!pte->present) {
    if (!create) {
      return 0;
    }

    pd[pdi(vaddr)] = allocate_page();
    pte->present = 1;
    pte->read_write = 1;
  }
  uintptr_t *pt = (uintptr_t *)(pd[pdi(vaddr)] & ~0xfff);
  return pt[pti(vaddr)];
}

void paging_init(uintptr_t start, size_t size) {
  free_ptr = (struct node *)start;
  num_pages = size / 4;

  // todo: going to fault here since most of memory is not yet mapped...
  // figure out how to set up pmm stack given ^
  struct node *prev = free_ptr;
  for (size_t i = 1; i < num_pages; ++i) {
    end = start + i * PAGE_SIZE_BYTES;
    prev->next = (struct node *)end;
    prev = prev->next;
    PRINTF("marked %lx\n", end)
  }

  pd = (uintptr_t *)allocate_page();
  PRINTF("paging init, free_ptr: %lx, num_pages: %lx, pd: %lx\n",
         (uintptr_t)free_ptr, num_pages, pd)
}

void pmap(uintptr_t vaddr, uintptr_t paddr, bool writable, bool user) {
  invlpg(vaddr);
  struct pte *pte = (struct pte *)get_pte(vaddr, true);
  pte->present = 1;
  pte->read_write = writable ? 1 : 0;
  pte->user_super = user ? 1 : 0;
  pte->frame = (uintptr_t)((paddr & ~0xfff) >> 12);
}

void punmap(uintptr_t vaddr) {
  uintptr_t *pte = (uintptr_t *)get_pte(vaddr, false);
  if (pte == 0) {
    return;
  }

  *pte = 0;
  invlpg(vaddr);
}

// first-fit heap
// todo: not first-fit heap
void heap_init() {}
