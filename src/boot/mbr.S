// mbr should relocate itself and bring in a bootloader
// from an active partition. in our simple system, we'll
// just load the first (and presumably only) active partition
.code16
mbr_start:
  cli
  // zero seg regs
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %sp
  mov %ax, %es
relocate:
  // bios loads us at 0x7c00
  // relocate ourselves (0x0600 seems nice)
  // and load vbr to 0x7c00
  mov $0x0100, %cx
  mov $0x7c00, %si
  mov $0x0600, %di
  rep movsw
  ljmp $0x0, $(start_check_partition)

start_check_partition:
  mov %dl, (drive_number)
  leaw partition1, %bx
  mov $4, %cx
check_partition:
  // check for activeness
  testb $0x80, (%bx)
  jnz found_boot_partition
  add $0x10, %bx
  dec %cx
  jnz check_partition
  // for now, assume there's a bootable partition
  // and fail hard if we don't find it
  jmp hang
found_boot_partition:
  movw %bx, (boot_partition)
  movb $0x10, (dap)
  movl $0x7c00, (dap_dest_addr_offset)
  // pte lba is at offset 0x08
  add $8, %bx
  movl (%bx), %ebx
  mov %ebx, (dap_lba)
  // read one block
  movw $1, (dap_blocks)
  // retry once
  mov $2, %cx
read_sector:
  mov (drive_number), %dl
  lea dap, %si
  // extended (lba) read
  mov $0x42, %ah
  int $0x13
  jnc boot
  dec %cx
  jz hang
  // else reset disk and try again
  xor %ah, %ah
  int $0x13
  jnc read_sector
  jmp hang
boot:
  // check for magic in boot image
  mov 0x7dfe, %bx
  cmp $0xaa55, %bx
  jne hang
  // todo: do we wanna hand off boot pte?
  movw (boot_partition), %si
  mov (drive_number), %dl
  // hold onto ur butts
  ljmp $0x0, $0x7c00

hang:
  jmp hang

drive_number:
  .byte 0

boot_partition:
  .word 0
  .word 0

// disk address packet
dap:
  // packet size
  .byte 0
  // reserved
  .byte 0
dap_blocks:
  .word 0
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

// partition table starts at offset 0x01be
// could also skip 0x01b4 and include optional
// 10 byte (8b + 2b reserved) disk id
.skip 0x01be-(.-mbr_start), 0

// our (only) active partition
// these are dummy values (a partition
// starting on the next sector)
// we'll drop these in favor of what
// mkimg ends up putting here
partition1:
  // active
  .byte 0x80
// chs addr of first sector
partition1_start_chs:
  .byte 0x00
  .byte 0x01
  .byte 0x00
// type (fat32 lba)
partition1_type:
  .byte 0x0c
// chs addr of last sector (650/4/63)
partition1_end_chs:
  .byte 0x04
  .byte 0xFF
  .byte 0x8a
// lba of first sector
partition1_start_lba:
  .byte 0x01
  .byte 0x00
  .byte 0x00
  .byte 0x00
// num sectors (163840)
partition1_size:
  .byte 0x00
  .byte 0x80
  .byte 0x02
  .byte 0x00
// partitions 2-4
.skip 48, 0

.word 0xaa55
