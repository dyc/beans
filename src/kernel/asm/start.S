.set MAGIC,    0xE85250D6
.set ISA,      0
.set LENGTH,   0x10
.set CHECKSUM, -(MAGIC + ISA + LENGTH)
.set KERNEL_PDI, (0xc0000000 >> 22) // 768th PD entry
.set READ_WRITE_PRESENT, 0x00000003

.section .multiboot2, "a", @progbits
.align 4
.long MAGIC
.long ISA
.long LENGTH
.long CHECKSUM

.align 4096
kernel_page_table:
.skip 4096
kernel_page_table_end:

.global kernel_page_directory
kernel_page_directory:
.skip 4096

.section .prelude, "ax", @progbits
.align 16
.global _start
.type _start, @function
_start:
      mov $READ_WRITE_PRESENT, %edx
      lea kernel_page_table, %ecx
  map_kernel:
      mov %edx, (%ecx)
      add $0x1000, %edx
      add $0x4, %ecx
      cmp $kernel_page_table_end, %ecx
      jl map_kernel

      mov $KERNEL_PDI, %edx
      lea kernel_page_directory, %ecx
      mov $kernel_page_table, (%ecx)
      orl $READ_WRITE_PRESENT, (%ecx)
      mov $kernel_page_table, (%ecx, %edx, 4)
      orl $READ_WRITE_PRESENT, (%ecx, %edx, 4)
      mov %ecx, %cr3

      // enable paging
      mov %cr0, %ecx
      or $0x80010000, %ecx
      mov %ecx, %cr0

      lea _higher_half_start, %ecx
      jmp *%ecx

.section .text
.align 4
_higher_half_start:
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
