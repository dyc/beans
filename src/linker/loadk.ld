OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x8600;
  .text : {
    *(.text.loadk);
    *(.text);
  }

  .data : {
    *(.rodata .data .bss);
  }
}
