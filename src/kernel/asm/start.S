.set MAGIC,    0xE85250D6
.set ISA,      0
.set LENGTH,   0x10
.set CHECKSUM, -(MAGIC + ISA + LENGTH)
.set KERNEL_PAGE_NUMBER, (0xc0000000 >> 22) // 768th PD entry
.set READ_WRITE_PRESENT, 0x00000083

.section .multiboot2, "a", @progbits
.align 4
.long MAGIC
.long ISA
.long LENGTH
.long CHECKSUM

.align 4096
kernel_page_directory:
// identity map first mb, containing boot info
.long READ_WRITE_PRESENT
.skip (KERNEL_PAGE_NUMBER - 1) * 4, 0
// identity map kernel
.long READ_WRITE_PRESENT
.skip (1024 - KERNEL_PAGE_NUMBER - 1) * 4, 0

.section .prelude, "ax", @progbits
.align 16
.global _start
.type _start, @function
_start:
      lea kernel_page_directory, %ecx
      mov %ecx, %cr3

      // todo: check if the next two blocks are actually what we want
      // enable 4mb pages
      mov %cr4, %ecx
      orl $0x00000010, %ecx
      mov %ecx, %cr4

      // enable paging
      mov %cr0, %ecx
      or $0x80010000, %ecx
      mov %ecx, %cr0

      lea _higher_half_start, %ecx
      jmp *%ecx

.section .text
.align 4
_higher_half_start:
      // todo: add this back
      // clear identity mapping
      // movl $0x0, kernel_page_directory

      // flush tlb
      mov %cr3, %ecx
      mov %ecx, %cr3

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

.section .bss, "aw"
.align 16
stack_bottom:
.skip 16384
stack_top:
