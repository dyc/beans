// mbr should relocate itself and bring in a bootloader
// from an active partition. in our simple system, we'll
// just load the first (and presumably only) active partition
.code16
mbr_start:
  cli
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
  ljmp $0x0, $start_check_partition
start_check_partition:
  mov %dl, (drive_number)
  lea partition1, %bx
  mov $4, %cx
check_partition:
  // for now, assume there's a bootable partition
  // and fail hard if we don't find it
  testb $0x80, (%bx)
  jnz found_boot_partition
  add $0x10, %bx
  dec %cx
  jnz check_partition
  jmp hang
found_boot_partition:
  mov %bx, (boot_partition)
  movl $0x7c00, (dap_dest_addr_offset)
  movl 8(%bx), %ebx // partition's lba
  mov %ebx, (dap_lba)
  mov $2, %cx
eread:
  lea dap, %si
  mov (drive_number), %dl
  mov $0x42, %ah
  int $0x13
  jnc boot
  dec %cx
  jz hang
  xor %ah, %ah
  int $0x13
  jnc eread
  jmp hang
boot:
  mov MAGIC_ADDR, %bx
  cmp $0xaa55, %bx
  jne hang
  mov (boot_partition), %si
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

dap: // disk address packet
  .byte 0x10 // packet size
  .byte 0 // reserved
dap_blocks:
  .byte 0x01
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
  .byte 0x80 // active
partition1_start_chs: // chs of first sector
  .byte 0x00
  .byte 0x01
  .byte 0x00
partition1_type: // type
  .byte 0x0c // (fat32 lba)
partition1_end_chs: // chs of last sector (650/4/63)
  .byte 0x04
  .byte 0xFF
  .byte 0x8a
partition1_start_lba:
  .byte 0x01
  .byte 0x00
  .byte 0x00
  .byte 0x00
partition1_size: // 163840
  .byte 0x00
  .byte 0x80
  .byte 0x02
  .byte 0x00
.skip 48, 0 // partitions 2-4

.set MAGIC_ADDR, 0x7dfe

.word 0xaa55
