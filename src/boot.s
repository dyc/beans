# taken from https://wiki.osdev.org/Bare_Bones
# which has lots of useful explanations

# constants for multiboot header
# https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Header-magic-fields
# boot modules must be (4kb) page-aligned
.set ALIGN,    1<<0
# include info on available memory
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

# declaration of multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# allocate a small (16kib) stack
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
      # yay stack pointer
      mov $stack_top, %esp
      call _init
      call kernel_main
      call _fini
      cli
  1:  hlt
      jmp 1b
.size _start, . - _start
