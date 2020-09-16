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
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %ss
  // big ol stack (bootloader occupies 0x7c00 - 0x7e00)
  mov $0x7c00, %ax
  mov %ax, %sp
  // e820 mmap will be directed here (0x500 since es is seg reg)
  // (bottom of conventional memory, which is the first mb)
  // x/120xb 0x500 to inspect
  mov $0x50, %ax
  mov %ax, %es
  // mbr hands us these
  movw %si, (boot_partition)
  mov %dl, (drive_number)

  // get available memory below 0x0A0000 (640kb)
  // error status returned in cf so clear it
  clc
  // returns kb from 0 (ivt and bda in first ~1kb)
  // in %ax (probably 639, ~1kb for ebda)
  int $0x12
  // stop if error
  jc hang
  mov %ax, (lower_mem)
  // find mmap
  call e820_first
  jc hang

  cli

  // fast a20 enable, if necessary
  in $0x92, %al
  testb $2, %al
  jnz no92
  or $2, %al
  and $0xfe, %al
  out %al, $0x92
no92:

  // load gdt
  xor %eax, %eax
  mov %ds, %ax
  shl $4, %eax
  add $gdt_start, %eax
  // init offset and size in descriptor
  mov %eax, gdt_descriptor + 2
  mov $gdt_end, %eax
  sub $gdt_start, %eax
  mov %ax, gdt_descriptor
  lgdtw gdt_descriptor

  // i guess mbr could have loaded this alongside us but
  // i like this better whatever
  movb $0x10, (dap)
  // load right after this
  mov $0x7e00, (dap_dest_addr)
  movw (boot_partition), %bx
  // lba is at offset 0x08, put it into dap
  add $8, %bx
  // read third block (mbr is 1, we are 2)
  add $2, (%bx)
  movl (%bx), %ebx
  mov %ebx, (dap_lba)
  mov $4, (dap_blocks)
  // retry once
  mov $2, %cx
read_sector:
  mov (drive_number), %dl
  lea dap, %si
  // extended (lba) read
  mov $0x42, %ah
  int $0x13
  jnc done_read
  dec %cx
  jz hang
  // else reset disk and try again
  xor %ah, %ah
  int $0x13
  jnc read_sector
  jmp hang

done_read:
  // protected mode
  // do after gdt with no interrupts
  mov %cr0, %eax
  or $1, %al
  mov %eax, %cr0

  // flush data segregs
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  mov %ax, %ss

  // flush code segreg
  ljmp $0x08, $0x7e00

// if first iteration fails then e820 isn't supported
// and we should indicate fail on return
e820_first:
  xor %ebx, %ebx
  xor %di, %di
  // magic ("smap")
  mov $0x534d4150, %edx
  mov $0xe820, %eax
  // let's play along with acpi 3.0 compliance
  movl $0x1, %es:20(%di)
  // request 24 bytes
  mov $24, %ecx
  int $0x15
  jc e820_fail
  mov $0x534d4150, %edx
  cmp %edx, %eax
  jne e820_fail
  // check cf, magic in eax, nonzero ebx for success
  jc e820_fail
  mov $0x534d4150, %edx
  cmp %edx, %eax
  jne e820_fail
  test %ebx, %ebx
  je e820_fail
  jmp e820_maybe_ingest

e820_next:
  mov $0x534d4150, %edx
  mov $0xe820, %eax
  movl $0x1, %es:20(%di)
  mov $24, %ecx
  int $0x15
  // doneness
  jc e820_done
  test %ebx, %ebx
  je e820_done

e820_maybe_ingest:
  // length returned in cx
  jcxz e820_next_or_done
  // if we got 20 bytes, try ingest
  cmp $20, %cl
  je e820_ingest
  // got 24 bytes (acpi 3.0), check skip bit
  testb $1, %es:20(%di)
  je e820_next_or_done

e820_ingest:
  mov %es:8(%di), %ecx
  or %es:12(%di), %ecx
  jz e820_next_or_done
  // track number of entries
  incb e820_count
  // prepare for next entry
  // most likely 20 byte entries but
  // inc by 24 fOR cOmPatiBIliTY
  add $24, %di

e820_next_or_done:
  test %ebx, %ebx
  jne e820_next

e820_done:
  clc
  ret

e820_fail:
  stc
  ret

hang:
  jmp hang

.global e820_count
e820_count:
  .byte 0

.global lower_mem
lower_mem:
  .byte 0
  .byte 0

gdt_descriptor:
  .word 0
  .long 0

gdt_start:
  // first entry unused
  .quad 0
  // code descriptor
  // limit, bits 0-15
  .word 0xffff
  // base, bits 0-15
  .word 0
  // base, bits 16-23
  .byte 0
  // access: read-enabled kcode segment
  .byte 0x9a
  // flags in high 4:
  // - granularity=1: limit is 4kb pages
  // - size=1: 32 bit operands
  // limit, bits 16-20 in low 4
  .byte 0xcf
  // base, bits 24-31
  .byte 0
  // data descriptor
  .word 0xffff
  .word 0
  .byte 0
  // access: write-enabled kdata segment
  .byte 0x92
  .byte 0xcf
  .byte 0
gdt_end:

.global drive_number
drive_number:
  .byte 0

.global boot_partition
boot_partition:
  .word 0
  .word 0

// todo: verify this is how we want to be doing reads...
dap:
  .byte 0
  .byte 0
dap_blocks:
  .word 0
dap_dest_addr:
  .word 0
  .word 0
dap_lba:
  .word 0
  .word 0
dap_lba_ext:
  .word 0
  .word 0

.skip 510-(.-_start), 0
.word 0xaa55
