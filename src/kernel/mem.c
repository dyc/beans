#include <stdint.h>

#include <kernel/mem.h>

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) gdtr_t;

extern void load_gdt(uintptr_t);

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
