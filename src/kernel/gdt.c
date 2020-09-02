#include <stddef.h>
#include <stdint.h>

struct gdt_descriptor {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t limit_high : 4;
  uint8_t flags : 4;
  uint8_t base_high;
} __attribute__((packed));

static struct {
  struct gdt_descriptor gdtr;
  struct gdt_entry entries[3];
} gdt __attribute__((used));

static void set_segment(
  size_t segment,
  uint64_t base,
  uint64_t limit,
  uint8_t access,
  uint8_t flags
) {
  gdt.entries[segment].base_low = base & 0xFFFF;
  gdt.entries[segment].base_mid = (base >> 16) & 0xFF;
  gdt.entries[segment].base_high = (base >> 24) & 0xFF;
  gdt.entries[segment].limit_low = limit & 0xFFFF;
  gdt.entries[segment].limit_high = (limit >> 16) & 0x0F;
  gdt.entries[segment].access = access;
  gdt.entries[segment].flags = flags & 0x0F;
}

extern void load_gdt(uintptr_t);

void gdt_install() {
  gdt.gdtr.limit = sizeof(gdt.entries) - 1;
  gdt.gdtr.base = (uintptr_t) &gdt.entries[0];
  // null segment
  set_segment(0, 0, 0, 0, 0);
  // kcode segment
  set_segment(1, 0, 0xFFFFFFFF, 0x9A, 0x0C);
  // kdata segment
  set_segment(2, 0, 0xFFFFFFFF, 0x92, 0x0C);

  load_gdt((uintptr_t) &gdt.gdtr);
}
