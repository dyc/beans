.set MAGIC,    0xE85250D6
.set ISA,      0
.set LENGTH,   0x10
.set CHECKSUM, -(MAGIC + ISA + LENGTH)

.section .multiboot2
.align 4
.long MAGIC
.long ISA
.long LENGTH
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
      mov $stack_top, %esp

      cli
      push %eax // caller-saved
      call _init
      pop %eax
      push %eax // multiboot eax magic
      push %ebx // multiboot info pointer
      call kmain
      call _fini
      cli

  1:  hlt
      jmp 1b
.size _start, . - _start
