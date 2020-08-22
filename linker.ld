ENTRY(_start)
SECTIONS {
  /* start sections at 1mib. apparently this is the place */
  . = 1M;

  /* multiboot header first */
  .text BLOCK(4K) : ALIGN(4K) {
    *(.multiboot)
    *(.text)
  }

  .rodata BLOCK(4K) : ALIGN(4K) {
    *(.rodata)
  }

  .data BLOCK(4K) : ALIGN(4K) {
    *(.data)
  }

  .bss BLOCK(4K) : ALIGN(4K) {
    *(COMMON)
    *(.bss)
  }
}
