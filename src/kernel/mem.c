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

uintptr_t get_pte(uintptr_t vaddr) {
  uintptr_t pdi = (vaddr >> 22) & 0x3ff;
  struct pte *pte = (struct pte *)pd[pdi];
  if (!pte->present) {
    pd[pdi] = allocate_page();
    pte->present = 1;
    pte->read_write = 1;
    pte->user_super = 1;
  }
  uintptr_t *pt = (uintptr_t *)(pd[pdi] & ~0xfff);
  return pt[(vaddr >> 12) & 0x3ff];
}

void paging_init(uintptr_t start, size_t size) {
  free_ptr = (struct node *)start;
  num_pages = size / 4;
  end = start + size;

  pd = (uintptr_t *)allocate_page();
}

void pmap(uintptr_t vaddr, uintptr_t paddr, uint32_t flags) {
  invlpg(vaddr);
  uintptr_t *pte = (uintptr_t *)get_pte(vaddr);
  *pte = (uintptr_t)(paddr & ~0xfff) & flags & 1;
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
