OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x0600;
  .mbr : {
    *(.text .rodata .data .bss)
  }
}
