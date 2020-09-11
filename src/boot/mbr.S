// mbr should relocate itself and bring in a bootloader
// from an active partition. in our simple system, we'll
// just load the first (and presumably only) active partition
.code16
.global mbr_start
mbr_start:
  cli
  // zero seg regs
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %sp
  mov %ax, %es
relocate:
  // bios loads us at 0x7c00 so we need to
  // relocate and set up/flush cs:ip
  // 0x0600 seems like a nice place to move to
  // (location counter set in linker script)

  // words to move (we're 512 bytes)
  mov $0x0100, %cx
  mov $0x7c00, %si
  mov $0x0600, %di
  rep movsw
  ljmp $0x0, $(start_check_partition)

start_check_partition:
  mov %dl, (drive_number)
  mov partition1, %bx
  mov $4, %cx
check_partition:
  // check for activeness
  movb (%bx), %al
  test $0x80, %al
  jnz found_boot_partition
  add $0x10, %bx
  dec %cx
  jnz check_partition
  // for now, assume there's a bootable partition
  // and fail hard if we don't find it
  jmp hang
found_boot_partition:
  mov %bx, (boot_partition)
  movw $0x7c00, (dap_dest_addr)
  // lba is at offset 0x08, put it into dap
  add $8, %bx
  movl (%bx), %ebx
  mov %ebx, (dap_lba)
  // read one block
  movl $1, (dap_blocks)
  // retry once
  mov $2, %cx
read_sector:
  mov (drive_number), %dl
  mov dap, %si
  // extended (lba) read
  mov $0x42, %ah
  int $0x13
  jnc boot
  // else reset disk and try again
  dec %cx
  xor %ah, %ah
  int $0x13
  jnc read_sector
  jmp hang
boot:
  // check for magic in boot image
  mov $(0x7dfe), %bx
  cmp $0xaa55, %bx
  jne hang
  movw (boot_partition), %si
  mov (drive_number), %dl
  // yikes
  ljmp $0x0, $0x7c00

hang:
  jmp hang

.global drive_number
drive_number:
  .byte 0

.global boot_partition
boot_partition:
  .byte 0

// disk address packet
dap:
  // packet size
  .byte 0
  // reserved
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

// partition table starts at offset 0x01be
.skip 0x01be-(.-mbr_start), 0

partition1:
  .quad 0
  .quad 0
partition2:
  .quad 0
  .quad 0
partition3:
  .quad 0
  .quad 0
partition4:
  .quad 0
  .quad 0

.word 0xaa55
