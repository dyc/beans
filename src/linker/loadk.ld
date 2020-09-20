OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x8400;
  .loadk : {
    *(.text .rodata .data .bss)
  }
}
