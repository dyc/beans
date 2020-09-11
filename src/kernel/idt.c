#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct idt_descriptor {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t flags;
  uint16_t offset_high;
} __attribute__((packed)) idt_entry;

static struct {
  struct idt_descriptor idtr;
  struct idt_entry entries[256];
} idt __attribute__((used));

void idt_set_gate(size_t gate, uint32_t offset, uint16_t selector,
                  uint8_t flags) {
  idt.entries[gate].offset_low = offset & 0xFFFF;
  idt.entries[gate].offset_high = (offset >> 16) & 0xFFFF;
  idt.entries[gate].selector = selector;
  idt.entries[gate].zero = 0;
  idt.entries[gate].flags = flags | 0x60;
}

extern void load_idt(uintptr_t);

void idt_install() {
  idt.idtr.limit = sizeof(idt.entries) - 1;
  idt.idtr.base = (uint32_t)&idt.entries[0];

  size_t n = sizeof(idt.entries) / sizeof(struct idt_entry);
  for (size_t i = 0; i < n; ++i) {
    idt_set_gate(i, 0, 0, 0);
  }

  load_idt((uintptr_t)&idt.idtr);
}
