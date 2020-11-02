ENTRY(_start)
SECTIONS {
  . = 1M;

  .multiboot2 : {
    *(.multiboot2)
  }

  .prelude : {
    *(.prelude)
  }

  . += 0xc0000000;
  _ld_kernel_phys_start = . - 0xc0000000;
  _ld_kernel_virt_start = .;
  .text ALIGN(4K) : AT (ADDR (.text) - 0xc0000000) {
    *(.text)
  }

  .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xc0000000) {
    *(.rodata)
  }

  .data ALIGN(4K) : AT (ADDR (.data) - 0xc0000000) {
    *(.data)
  }

  .bss ALIGN(4K) : AT (ADDR (.bss) - 0xc0000000) {
    *(COMMON)
    *(.bss)
  }
  _ld_kernel_phys_end = . - 0xc0000000;
  _ld_kernel_virt_end = .;

  /DISCARD/ : {
    *(.comment)
  }
}
