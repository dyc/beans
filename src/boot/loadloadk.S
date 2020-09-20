// org 0x8200 (from linker script)

// given addr of bpb/ebpb in %bx,
// loads /boot/loadk into memory at %di
.code16
loadloadk:
  movw %di, (dap_dest_addr_offset)
  movb %al, (drive_number)

  xor %ax, %ax
  xor %cx, %cx
  xor %dx, %dx

  // bpb_fats
  movb 0x10(%bx), %al
  // ebpb_sectors_per_fat
  mul 0x24(%bx)
  // bpb_reserved_sectors (typically 32)
  addw 0x0e(%bx), %ax
  // account for partition LBA offset i.e. bpb_hidden_sectors
  // could also use pte's lba: 8(pte)
  addl 0x1c(%bx), %eax
  movw %ax, %cx

  // relative offset (in sectors) of root cluster
  movl 0x2c(%bx), %eax
  sub $2, %eax
  // bpb_sectors_per_cluster
  movb 0x0d(%bx), %dl
  mulb %dl
  // lba of root cluster
  addl %ecx, %eax
  mov %eax, (dap_lba)

  movb $0x10, (dap)
  movw $1, (dap_blocks)
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
  // find loadk in root cluster
  ret

hang:
  jmp hang

drive_number:
  .byte 0

dap:
  .byte 0
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

loadk_target: .string "loadk"
.skip 512-(.-loadloadk), 0
