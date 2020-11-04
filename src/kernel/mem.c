#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

// todo: delete from here
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

// kernel pd is initialized to the temporary pd we use to higher half
// ourselves. we'll be using it to bootstrap pmm after which it'll be
// reinstantiated via pmm.
extern uintptr_t kernel_page_directory;
uintptr_t *pd = &kernel_page_directory;

uintptr_t start_allocate_page() {
  return free_ptr == NULL || (uintptr_t)free_ptr == end ? 0
                                                        : (uintptr_t)free_ptr;
}

uintptr_t finish_allocate_page() {
  uintptr_t ret = (uintptr_t)free_ptr;
  free_ptr = free_ptr->next;
  return ret;
}

void start_deallocate_page(uintptr_t addr) {
  struct node *n = (struct node *)addr;
  n->next = free_ptr;
  free_ptr = n;
}

void finish_deallocate_page(uintptr_t addr) {
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
  if (!pte->present) {
    if (!create) {
      return 0;
    }

    pd[pdi(vaddr)] = start_allocate_page();
    // identity map this pt here?
    finish_allocate_page();
    pte->present = 1;
    pte->read_write = 1;
  }
  uintptr_t *pt = (uintptr_t *)(pd[pdi(vaddr)] & ~0xfff);
  return pt[pti(vaddr)];
}

void paging_init(uintptr_t start, size_t size) {
  free_ptr = (struct node *)start;
  num_pages = size / 4;

  // set up free page linked list for pmm. since paging is enabled by this
  // point, we'll need to prop up a temporary kernel pd and map pages as we
  // mark them as free. we can use first free page to back these mappings.
  // immediately afterwards, we'll toss this bootstrap for our actual kernel
  // pd.
  uintptr_t *pt = &start;

  // todo: definitely missing something obvious here...we get garbage in n here
  // using an int works?
  struct node *n = (struct node *)(start + PAGE_SIZE_BYTES);
  for (size_t i = 1; i < num_pages; ++i) {
    end = (uintptr_t)n + PAGE_SIZE_BYTES;
    // todo: accommodate dma
    pd[pdi(start)] = *pt;
    pt[pti(start)] = ((uintptr_t)n & ~0x3ff) | 0x3;

    n->next = (struct node *)end;
    n = n->next;
  }
  PRINTF("initialized pmm ll with %lx pages\n", num_pages)
  // todo: figure out how to reclaim the first page?

  pd = (uintptr_t *)start_allocate_page();
  finish_allocate_page();
  PRINTF("done paging init\n")
  PRINTF("free_ptr %lx num_pages %lx pd %lx\n", (uintptr_t)free_ptr, num_pages,
         pd)
  // todo: set_crt3(pd);
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
