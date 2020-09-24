// given addr of bpb/ebpb in %bx,
// and driver number in %al
// loads /boot/loadk into memory at %di
// clobbery :'-)
// org 0x8200 (from linker script)
.code16
loadloadk:
  pop (e820_count)
  pop (drive_number)
  popw %bx
  movw $0x8400, (dap_dest_addr_offset)
  xor %ax, %ax
  // clear ds, es for cmpstr
  mov %ax, %ds
  mov %ax, %es
load_root_cluster:
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
  call eread_start
find_boot_dir:
  mov (dap_dest_addr_offset), %si
  mov $boot_dir, %di
find_boot_dir_loop:
  movb $boot_dir_strlen, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
load_boot_cluster:
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
  call eread_start
  // find each of loadk, kernel and ramdisk
  mov $3, %dh
find_files:
  mov (dap_dest_addr_offset), %si
find_files_loop:
  mov $loadk_file, %di
  movb $loadk_file_strlen, %cl
  call cmpstr
  je found_file_common

  mov $kernel_file, %di
  movb $kernel_file_strlen, %cl
  call cmpstr
  je found_file_common

  mov $ramdisk_file, %di
  movb $ramdisk_file_strlen, %cl
  call cmpstr
  je found_file_common

  // no match
  add $0x20, %si
  jmp find_files_loop
found_file_common:
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
  // todo: this
  // get size in sectors
  movl 0x1c(%si), %ecx
  shr $9, %ecx
found_loadk:
  mov %eax, (loadk_lba)
  mov %ecx, (loadk_size)
  jmp find_next_or_done
found_kernel:
  mov %eax, (kernel_lba)
  mov %ecx, (kernel_size)
  jmp find_next_or_done
found_ramdisk:
  mov %eax, (ramdisk_lba)
  mov %ecx, (ramdisk_size)
  jmp find_next_or_done
find_next_or_done:
  dec %dh
  jz load_loadk
  add $0x20, %si
  jmp find_files
// todo: use size to calculate dest addr for each of these
load_loadk:
  mov (loadk_lba), %eax
  mov %eax, (dap_lba)
  call eread_start
load_kernel:
  mov (kernel_lba), %eax
  mov %eax, (dap_lba)
  call eread_start
load_ramdisk:
  mov (ramdisk_lba), %eax
  mov %eax, (dap_lba)
  call eread_start

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
  pushl $0x500
  xor %eax, %eax
  mov (e820_count), %al
  push %eax
  pushl $hang
  // flush and jmp to loaded loadk
  ljmp $0x08, $0x8400

eread_start:
  mov $2, %cx
eread:
  lea dap, %si
  mov (drive_number), %dl
  mov $0x42, %ah
  int $0x13
  jnc eread_done
  dec %cx
  jz hang
  // reset and try again
  xor %ah, %ah
  int $0x13
  jnc eread
  jmp hang
eread_done:
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
  .byte 0x10
  .byte 0
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

e820_count:
  .byte 0

// save the original lba so we can
// use it to calculate correct offsets
// when/if dap_lba is ever overwritten
root_cluster_lba:
  .word 0
  .word 0
loadk_lba:
  .word 0
  .word 0
kernel_lba:
  .word 0
  .word 0
ramdisk_lba:
  .word 0
  .word 0
loadk_size:
  .byte 0
kernel_size:
  .byte 0
ramdisk_size:
  .byte 0

boot_dir: .ascii "BOOT"
.equ boot_dir_strlen, .-boot_dir
loadk_file: .ascii "LOADK"
.equ loadk_file_strlen, .-loadk_file
// todo: maybe get this from a config instead
kernel_file: .ascii "BEANS"
.equ kernel_file_strlen, .-kernel_file
ramdisk_file: .ascii "RAMDISK"
.equ ramdisk_file_strlen, .-ramdisk_file
.skip 512-(.-loadloadk), 0
