# taken from https://wiki.osdev.org/Bare_Bones
# which has lots of useful explanations

# constants for multiboot header
# https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Header-magic-fields
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
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
