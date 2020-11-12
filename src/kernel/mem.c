#include <stddef.h>
#include <stdint.h>

#include <kernel/mem.h>

// todo: delete from here
#include <kernel/macros.h>
static char buf[256] = {0};
// todo: to here

const uint32_t PAGE_SIZE_BYTES = 0x1000;

struct node {
  struct node *next;
};

struct node *free_ptr = NULL;
uintptr_t end;

// kernel pd is initialized to the temporary pd we use to higher half
// ourselves. we'll be using it to bootstrap pmm after which it'll be
// reinstantiated via pmm.
extern uintptr_t kernel_page_directory;
extern uintptr_t kernel_page_table;
uintptr_t *pd = &kernel_page_directory;

// todo: may need to break this up into two stages
uintptr_t allocate_page() {
  if (free_ptr == NULL || (uintptr_t)free_ptr == end) {
    return 0;
  }

  uintptr_t ret = (uintptr_t)free_ptr;
  free_ptr = free_ptr->next;
  return ret;
}

void deallocate_page(uintptr_t addr) {
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

void set_cr3(uintptr_t addr) { asm volatile("mov %%eax, %%cr3" ::"a"(addr)); }

inline size_t pdi(uintptr_t vaddr) { return (vaddr >> 22) & 0x3ff; }

inline size_t pti(uintptr_t vaddr) { return (vaddr >> 12) & 0x3ff; }

uintptr_t *get_pte(uintptr_t vaddr, bool create) {
  struct pde *pde = (struct pde *)&pd[pdi(vaddr)];
  if (!pde->present) {
    if (!create) {
      return 0;
    }

    pde->present = 1;
    pde->read_write = 1;
    uintptr_t *pt = (uintptr_t *)allocate_page();
    memset(pt, 0, PAGE_SIZE_BYTES);
    pde->table = (uintptr_t)pt >> 12;
  }
  return &((uintptr_t *)(pde->table << 12))[pti(vaddr)];
}

void paging_init(uintptr_t start, size_t size) {
  free_ptr = (struct node *)(start + PAGE_SIZE_BYTES);
  end = start + size;
  PRINTF("initializing pmm at %lx with %lx bytes\n", start, size)

  // set up free page linked list for pmm. since paging is enabled by this
  // point, we'll need to prop temporary ptes to map pages as we mark them as
  // free. we can use first free page for this. immediately afterwards, we'll
  // toss this bootstrap for our actual kernel pd.
  uintptr_t *pt = (uintptr_t *)start;
  uintptr_t current = start + PAGE_SIZE_BYTES;
  for (; current < end; current += PAGE_SIZE_BYTES) {
    // todo: accommodate dma
    // use the existing kernel pt if pte is already set
    if (!pd[pdi(current)]) {
      struct pde *pde = (struct pde *)&pd[pdi(current)];
      pde->present = 1;
      pde->read_write = 1;
      pde->table = (uintptr_t)pt >> 12;
    }
    struct pte *pte = (struct pte *)&pt[pti(current)];
    pte->present = 1;
    pte->read_write = 1;
    pte->frame = (uintptr_t)current >> 12;
    *((uintptr_t *)current) = current + PAGE_SIZE_BYTES;
  }

  // recycle pt for kernel pt
  memcpy(pt, &kernel_page_table, PAGE_SIZE_BYTES);

  pd = (uintptr_t *)allocate_page();
  memset(pd, 0, PAGE_SIZE_BYTES);
  pd[0] = ((uintptr_t)pt & ~0xfff) | 0x3;
  pd[pdi((uintptr_t)&_ld_kernel_virt_start)] = ((uintptr_t)pt & ~0xfff) | 0x3;
  set_cr3((uintptr_t)pd);
  PRINTF("final kernel pt %lx and pd %lx\n", pt, pd)
}

void pmap(uintptr_t vaddr, uintptr_t paddr, bool writable, bool user) {
  invlpg(vaddr);
  struct pte *pte = (struct pte *)get_pte(vaddr, true);
  pte->present = 1;
  pte->read_write = writable ? 1 : 0;
  pte->user_super = user ? 1 : 0;
  pte->frame = (uintptr_t)paddr >> 12;
}

void punmap(uintptr_t vaddr) {
  uintptr_t *pte = get_pte(vaddr, false);
  if (pte == 0) {
    return;
  }

  *pte = 0;
  invlpg(vaddr);
}

// first-fit heap
// todo: not first-fit heap
void heap_init() {}
