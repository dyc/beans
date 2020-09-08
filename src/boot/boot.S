// note to self: dont forget to das as 16 bit:
// i686-elf-objdump -m i386 -M addr16,data16 -b binary -D build/bin/boot
.code16
.global _start
_start:
  // org 0x7c00 (from linker script)
  // real mode, cs:ip = (cs << 4) + ip
  // set cs to next instruction (ljmp is 5 bytes)
  ljmp $0x0000, $0x7c05

  // set up seg regs
  mov $0x7c00, %ax
  mov %ax, %ds

  // 256 byte stack (bootloader occupies 0x7c00 - 0x7e00)
  mov $0x7e00, %ax
  mov %ax, %ss
  mov $0x7f00, %ax
  mov %ax, %sp

  // get available memory ("conventional memory")
  // below 0x0A0000 (640kb)
  // error status returned in CF so clear it
  clc
  // returns kb from 0 (ivt and bda in first ~1kb)
  // in %ax (probably 639, ~1kb for ebda)
  int $0x12
  // stop if error
  jc hang
  mov %ax, (mb_mem_lower)

  cli
  jmp hang

hang:
  jmp hang

.global mb_mem_lower
mb_mem_lower:
  .byte 0
  .byte 0

.skip 510-(.-_start), 0
.word 0xaa55
