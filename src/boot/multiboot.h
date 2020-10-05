#ifndef BEANS_BOOT_MULTIBOOT_H_
#define BEANS_BOOT_MULTIBOOT_H_

#include <stdint.h>

const uint32_t MULTIBOOT_BOOTLOADER_MAGIC = 0x2BADB002;
const uint32_t MULTIBOOT_INFO_MODS = 0x00000008;
const uint32_t MULTIBOOT_MMAP_AVAILABLE = 1;
const uint32_t MULTIBOOT_MMAP_RESERVED = 2;
const uint32_t MULTIBOOT_MMAP_ACPI_RECLAIMABLE = 3;
const uint32_t MULTIBOOT_MMAP_ACPI_NVS = 4;
const uint32_t MULTIBOOT_MMAP_BAD = 5;

struct multiboot_aout_symbol_table {
  uint32_t tabsize;
  uint32_t strsize;
  uint32_t addr;
  uint32_t reserved;
};

struct multiboot_elf_section_header_table {
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;
};

struct multiboot_info {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  union {
    struct multiboot_aout_symbol_table aout_sym;
    struct multiboot_elf_section_header_table elf_sec;
  } u;
  uint32_t mmap_length;
  uint32_t mmap_addr;
  uint32_t drives_length;
  uint32_t drives_addr;
  uint32_t config_table;
  uint32_t boot_loader_name;
  uint32_t apm_table;
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;

  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2
  uint8_t framebuffer_type;
  union {
    struct {
      uint32_t framebuffer_palette_addr;
      uint16_t framebuffer_palette_num_colors;
    };
    struct {
      uint8_t framebuffer_red_field_position;
      uint8_t framebuffer_red_mask_size;
      uint8_t framebuffer_green_field_position;
      uint8_t framebuffer_green_mask_size;
      uint8_t framebuffer_blue_field_position;
      uint8_t framebuffer_blue_mask_size;
    };
  };
};

struct multiboot_module {
  uint32_t mod_start;
  uint32_t mod_end;
  uint32_t cmdline;
  uint32_t pad;
};

struct multiboot_mmap_entry {
  uint64_t base;
  uint64_t length;
  uint32_t type;
  uint32_t unused;
};

#endif
