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

.section .bss, "aw"
.align 16
stack_bottom:
.skip 16384
stack_top:
.align 4096
kernel_page_directory:
.skip 4096
kernel_page_table:
.skip 4096

.section .prelude
.global _start
.type _start, @function
_start:
      mov $(kernel_page_directory - 0xc0000000), %edi // phys addr of pd
      // look for kernel start, starting at 0
      mov $0x0, %esi
      mov $0x3ff, %ecx
map_kernel:
      cmp $(_kernel_start - 0xc0000000), %esi
      jl next_page
      cmp $(_kernel_end - 0xc0000000), %esi
      jge done_mapping_kernel
      mov %esi, %edx
      or $0x3, %edx
      mov %edx, (%edi)
next_page:
      add 0x1000, %esi
      add 0x4, %edi
      loop map_kernel
done_mapping_kernel:
      // map vga
      movl $(0x000b8000 | 0x3), kernel_page_table - 0xc0000000 + 0x3ff * 0x4
      // identity map kernel
      movl $(kernel_page_table - 0xc0000000 + 0x3), kernel_page_directory - 0xc0000000
      // map kernel to 0xc0000000
      movl $(kernel_page_table - 0xc0000000 + 0x3), kernel_page_directory - 0xc0000000 + 0x300 * 0x4
      // load kernel_page_directory
      movl $(kernel_page_directory - 0xc0000000), %ecx
      mov %ecx, %cr3
      // enable paging
      mov %cr0, %ecx
      or 0x80010000, %ecx
      mov %ecx, %cr0
      // jump to (now higher half) kernel
      lea real_start, %ecx
      jmp *%ecx

.section .text
real_start:
      // clear identity mapping
      movl $0, kernel_page_directory
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
