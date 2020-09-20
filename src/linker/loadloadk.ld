OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x8000;
  .loadloadk : {
    *(.text .rodata .data .bss)
  }
}
