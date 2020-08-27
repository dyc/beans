#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/mem.h>

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) gdtr_t;

// https://wiki.osdev.org/Global_Descriptor_Table
typedef struct {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t limit_high : 4;
  uint8_t flags : 4;
  uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

static struct {
  gdtr_t gdtr;
  gdt_entry_t entries[3];
} gdt;

static void set_segment(
  gdt_entry_t* segment,
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
) {
  segment->base_low = base & 0xFFFF;
  segment->base_mid = (base >> 16) & 0xFF;
  segment->base_high = (base >> 24) & 0xFF;
  segment->limit_low = limit & 0xFFFF;
  segment->limit_high = (limit >> 16) & 0x0F;
  segment->access = access;
  segment->flags = flags & 0x0F;
}

extern void load_gdt(uintptr_t);

void gdt_install() {
  gdtr_t* gdtr_p = &gdt.gdtr;
  gdtr_p->limit = sizeof(gdt.entries) - 1;
  gdtr_p->base = (uint32_t) &gdt.entries[0];

  // null segment
  set_segment(&gdt.entries[0], 0, 0, 0, 0);
  // code segment
  set_segment(&gdt.entries[1], 0, 0xFFFFFFFF, 0x9A, 0x0C);
  // data segment
  set_segment(&gdt.entries[2], 0, 0xFFFFFFFF, 0x92, 0x0C);

  load_gdt((uintptr_t) gdtr_p);
}

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr_t;

// https://wiki.osdev.org/Global_Descriptor_Table
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
  uint32_t selector,
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
