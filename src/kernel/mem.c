#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

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

uintptr_t get_pte(uintptr_t vaddr) {
  struct pte *pte = (struct pte *)pd[pdi(vaddr)];
  if (!pte->present) {
    pd[pdi(vaddr)] = allocate_page();
    pte->present = 1;
    pte->read_write = 1;
    pte->user_super = 1;
  }
  uintptr_t *pt = (uintptr_t *)(pd[pdi(vaddr)] & ~0xfff);
  return pt[pti(vaddr)];
}

void paging_init(uintptr_t start, size_t size) {
  free_ptr = (struct node *)start;
  num_pages = size / 4;
  end = start + size;

  pd = (uintptr_t *)allocate_page();
}

void pmap(uintptr_t vaddr, uintptr_t paddr, bool writable, bool user) {
  invlpg(vaddr);
  struct pte *pte = (struct pte *)get_pte(vaddr);
  pte->present = 1;
  pte->read_write = writable ? 1 : 0;
  pte->user_super = user ? 1 : 0;
  pte->frame = (uintptr_t)paddr & ~0xfff;
}

void punmap(uintptr_t vaddr) {
  uintptr_t *pte = (uintptr_t *)get_pte(vaddr);
  *pte = 0;
  invlpg(vaddr);
}

// first-fit heap
// todo: not first-fit heap
void heap_init() {}

uint32_t get_num_pages() { return num_pages; }
