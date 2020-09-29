#ifndef BEANS_BOOT_MEM_H_
#define BEANS_BOOT_MEM_H_

#include <stdint.h>

const uint32_t SMAP_TYPE_RAM = 1;
const uint32_t SMAP_TYPE_RESERVED = 2;
const uint32_t SMAP_TYPE_ACPI_RECLAIMABLE = 3;
const uint32_t SMAP_TYPE_ACPI_NVS = 4;

struct smap_entry {
  uint64_t base, length;
  uint32_t type, unused;
} __attribute__((packed));

#endif
