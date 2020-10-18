OUTPUT_FORMAT(binary)
SECTIONS {
  . = 0x10000;
  .text : {
    *(.text.loadk);
    *(.text);
  }

  .data : {
    *(.rodata .data .bss);
  }
}
