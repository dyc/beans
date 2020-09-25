// a fat32 bootloader
// org 0x7c00 (from linker script)
.code16
bpb_start:
  .byte 0
  .byte 0
  .byte 0
bpb_oem:
  .quad 0
bpb_bytes_per_sector:
  .word 0
bpb_sectors_per_cluster:
  .byte 0
bpb_reserved_sectors:
  .word 0
bpb_fats:
  .byte 0
bpb_dirs:
  .word 0
bpb_num_sectors_small:
  .word 0
bpb_desc_type:
  .byte 0
bpb_sectors_per_fat:
  .word 0
bpb_sectors_per_track:
  .word 0
bpb_heads:
  .word 0
bpb_hidden_sectors:
  .long 0
bpb_num_sectors_large:
  .long 0
ebpb_sectors_per_fat:
  .long 0
ebpb_flags:
  .word 0
ebpb_version:
  .word 0
ebpb_root_cluster:
  .long 0
ebpb_fsinfo_sector:
  .word 0
ebpb_backup_sector:
  .word 0
ebpb_reserved:
  .long 0
  .long 0
  .long 0
ebpb_drive_number:
  .byte 0
ebpb_flags_windows:
  .byte 0
ebpb_signature:
  .byte 0
ebpb_volume_id:
  .long 0
ebpb_volume_label:
  .quad 0
  .byte 0
  .byte 0
  .byte 0
ebpb_id:
  .quad 0

  // flush cs:ip
  ljmp $0x0000, $_start
_start:
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %ss
  // big ol stack below us with
  // stack base after bda and mbr
  mov $0x7c00, %ax
  mov %ax, %sp
  mov $0x800, %ax
  mov %ax, %bp
  // e820 mmap will be directed here
  // bottom of conventional memory
  mov $0x50, %ax
  mov %ax, %es
  // mbr hands us these
  movw %si, (partition_table_entry)
  mov %dl, (drive_number)

  // get available memory below 0x0A0000 (640kb)
  // returns kb from 0 (ivt and bda in first ~1kb)
  // in %ax (probably 639, ~1kb for ebda)
  int $0x12
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
  // lgdt
  xor %eax, %eax
  mov %ds, %ax
  shl $4, %eax
  add $gdt_start, %eax
  mov %eax, gdt_descriptor + 2
  mov $gdt_end, %eax
  sub $gdt_start, %eax
  mov %ax, gdt_descriptor
  lgdtw gdt_descriptor

  // read fsinfo block and loadloadk
  // into memory right after us
  movl $0x7e00, (dap_dest_addr_offset)
  // get this partition's lba offset
  mov (partition_table_entry), %bx
  add $8, %bx
  movl (%ebx), %eax
  // start read after boot (this) block
  addb $1, %al
  mov %eax, (dap_lba)
  mov $2, %cx
eread:
  lea dap, %si
  mov (drive_number), %dl
  mov $0x42, %ah
  int $0x13
  jnc jmp_loadloadk
  dec %cx
  jz hang
  // reset and try again
  xor %ah, %ah
  int $0x13
  jnc eread
  jmp hang
jmp_loadloadk:
  pushw $bpb_start
  push (drive_number)
  push (e820_count)
  jmp 0x8000

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
  jc hang
  mov $0x534d4150, %edx
  cmp %edx, %eax
  jne hang
  // check cf, magic in eax, nonzero ebx for success
  jc hang
  mov $0x534d4150, %edx
  cmp %edx, %eax
  jne hang
  test %ebx, %ebx
  je hang
  jmp e820_maybe_ingest
e820_next:
  mov $0x534d4150, %edx
  mov $0xe820, %eax
  movl $0x1, %es:20(%di)
  mov $24, %ecx
  int $0x15
  jc e820_done
  test %ebx, %ebx
  je e820_done
e820_maybe_ingest:
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
hang:
  jmp hang

e820_count:
  .byte 0

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

drive_number:
  .byte 0

partition_table_entry:
  .word 0
  .word 0

dap:
  .byte 0x10
  .byte 0
dap_blocks:
  .byte 0x02
  .byte 0
dap_dest_addr_offset:
  .word 0
dap_dest_addr_segment:
  .word 0
dap_lba:
  .word 0
  .word 0
dap_lba_ext:
  .word 0
  .word 0

.skip 510-(.-bpb_start), 0
.word 0xaa55
