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

  // todo: keep booting here
  cli
  mov $0xdead, %eax
  jmp hang

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

.skip 510-(.-_start), 0
.word 0xaa55
