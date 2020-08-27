#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr_t;

typedef struct {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t flags;
  uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

static struct {
  idtr_t idtr;
  idt_entry_t entries[256];
} idt;

static void set_gate(
  idt_entry_t* gate,
  uint32_t offset,
  uint16_t selector,
  bool present,
  uint8_t dpl,
  bool is_task,
  uint8_t type
) {
  gate->offset_low = offset & 0xFFFF;
  gate->selector = selector;
  gate->zero = 0;
  gate->flags = (present << 7) | (dpl << 5) | (is_task << 4) | (type & 0xFF);
  gate->offset_high = (offset >> 16) & 0xFFFF;
}

extern void load_idt(uintptr_t);

void idt_install() {
  idtr_t* idtr_p = &idt.idtr;
  idtr_p->limit = sizeof(idt.entries) - 1;
  idtr_p->base = (uint32_t) &idt.entries[0];

  size_t n = sizeof(idt.entries) / sizeof(idt_entry_t);
  for (size_t i = 0; i < n; ++i) {
    set_gate(&idt.entries[i], 0, 0, 0, 0, 0, 0);
  }

  load_idt((uintptr_t) idtr_p);
}
