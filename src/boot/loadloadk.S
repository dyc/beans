// org 0x8200 (from linker script)

// given addr of bpb/ebpb in %bx,
// and driver number in %al
// loads /boot/loadk into memory at %di
// clobbery :'-)
.code16
loadloadk:
  movw %di, (dap_dest_addr_offset)
  movb %al, (drive_number)

  xor %ax, %ax
  // clear ds, es for cmpstr
  mov %ax, %ds
  mov %ax, %es

read_root_cluster:
  // bpb_fats
  movb 0x10(%bx), %al
  // ebpb_sectors_per_fat
  mull 0x24(%bx)
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
  mov %eax, (root_cluster_lba)
  call read_sector

find_boot_dir:
  mov (dap_dest_addr_offset), %si
  mov $boot_dir, %di
find_boot_dir_loop:
  movb $boot_dir_strlen, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
read_boot_cluster:
  // get cluster number from dirent
  movw 0x14(%si), %ax
  shl $16, %eax
  movw 0x1a(%si), %ax
  sub $2, %eax
  // bpb_sectors_per_cluster
  movb 0x0d(%bx), %dl
  mulb %dl
  // we have lba of root cluster in dap_lba
  // so add lba(boot's cluster number)
  addl %eax, (dap_lba)
  call read_sector

find_loadk_file:
  mov (dap_dest_addr_offset), %si
  mov $loadk_file, %di
find_loadk_file_loop:
  movb $loadk_file_strlen, %cl
  call cmpstr
  je read_loadk
  add $0x20, %si
  jmp find_loadk_file_loop
read_loadk:
  // get cluster number from dirent
  movw 0x14(%si), %ax
  shl $16, %eax
  movw 0x1a(%si), %ax
  sub $2, %eax
  // bpb_sectors_per_cluster
  movb 0x0d(%bx), %dl
  mulb %dl
  // we have lba of root cluster in dap_lba
  // so add lba(loadk's cluster number)
  addl (root_cluster_lba), %eax
  mov %eax, (dap_lba)
  call read_sector

// at this point we'd loaded loadk to where %di was
// pointing. we're done :-)
// todo: might need utilize file size here if loadk gets large
loadloadk_done:
  ret

read_sector:
  // retry once
  mov $2, %cx
  movb $0x10, (dap)
  movw $1, (dap_blocks)
  mov (drive_number), %dl
  lea dap, %si
  // extended (lba) read
  mov $0x42, %ah
  int $0x13
  jnc read_done
  dec %cx
  jz hang
  // else reset disk and try again
  xor %ah, %ah
  int $0x13
  jnc read_sector
  jmp hang
read_done:
  ret

cmpstr:
  push %si
  push %di
  // let's have cld be default
  cld
  repe cmpsb
  pop %di
  pop %si
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

// save the original lba so we can
// use it to calculate correct offsets
// when/if dap_lba is ever overwritten
root_cluster_lba:
  .word 0
  .word 0

boot_dir: .ascii "BOOT"
.equ boot_dir_strlen, .-boot_dir
loadk_file: .ascii "LOADK"
.equ loadk_file_strlen, .-loadk_file
.skip 512-(.-loadloadk), 0
