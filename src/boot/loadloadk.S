// loads /boot/loadk, /boot/kernel, /boot/ramdisk
// into memory starting at $0x8400 (in that order)
// clobbery :'-)
// org 0x8200 (from linker script)
.code16
_start:
  popw (e820_count)
  popw (drive_number)
  pop %bx // bpb_start
  xor %eax, %eax
  // clear ds, es for cmpstr
  mov %ax, %ds
  mov %ax, %es

// assume everything we need is in the first fat
load_fat:
  movw $fat, (dap_dest_addr_offset)
  // account for partition LBA offset
  // could also use pte's lba: 8(pte)
  mov 0x1c(%bx), %eax // bpb_hidden_sectors
  // within partition, fat starts here
  add 0x0e(%bx), %ax // bpb_reserved_sectors (typically 32)
  mov %eax, (dap_lba) // fat lba
  call eread
  xor %eax, %eax
  mov 0x10(%bx), %al // bpb_fats
  mull 0x24(%bx) // ebpb_sectors_per_fat
  add %eax, (dap_lba) // root cluster lba
  // loads will go right after fat
  addw $sector_bytes, (dap_dest_addr_offset)

load_root_cluster:
  mov 0x2c(%bx), %eax // root cluster num
  call load_cluster
find_boot_dir:
  mov (dap_dest_addr_offset), %si
  mov $boot_dir, %di
find_boot_dir_loop:
  mov $boot_dir_strlen, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
  call load_cluster_from_dirent

  // find each of loadk, kernel and ramdisk
  mov $3, %dl
  mov (dap_dest_addr_offset), %si
check_loadk:
  mov $loadk_file, %di
  mov $loadk_file_strlen, %cl
  call cmpstr
  jne check_kernel
  call get_cluster_info
  mov %ax, (loadk_cluster)
  jmp maybe_check_next
check_kernel:
  mov $kernel_file, %di
  mov $kernel_file_strlen, %cl
  call cmpstr
  jne check_ramdisk
  call get_cluster_info
  mov %ax, (kernel_cluster)
  jmp maybe_check_next
check_ramdisk:
  mov $ramdisk_file, %di
  mov $ramdisk_file_strlen, %cl
  call cmpstr
  jne check_next
  call get_cluster_info
  mov %ax, (ramdisk_cluster)
  jmp maybe_check_next

// out
// - %ax: cluster num
// - %cx: size in sectors
get_cluster_info:
  push %dx
  xor %dx, %dx
  mov 0x1c(%si), %ax // size in bytes
  divw 0x0b(%bx) // bytes per sector
  cmp $0, %dx
  je no_remainder
  add $1, %ax
no_remainder:
  mov %ax, %cx
  mov 0x14(%si), %ax
  shl $16, %eax
  mov 0x1a(%si), %ax
  pop %dx
  ret
maybe_check_next:
  dec %dl
  jz load_start
check_next:
  add $0x20, %si
  jmp check_loadk

load_start:
  movw $loadk, (dap_dest_addr_offset)
load_loadk:
  mov (loadk_cluster), %ax
  call load_file_cluster
load_kernel:
  mov (kernel_cluster), %ax
  call load_file_cluster
load_ramdisk:
  mov (ramdisk_cluster), %ax
  call load_file_cluster

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
  pushl $mmap
  xor %eax, %eax
  mov (e820_count), %al
  push %eax
  pushl $hang
  // flush and jmp to loaded loadk
  ljmp $0x08, $loadk

// params
// - %si: dirent
// out
// - %eax: cluster
load_cluster_from_dirent:
  mov 0x14(%si), %ax
  shl $16, %eax
  mov 0x1a(%si), %ax
// params
// - %eax: cluster
// out
// - %eax: lba(cluster), relative to first cluster
load_cluster:
  pushl (dap_lba)
  push %eax
  sub $2, %eax // root cluster offset
  mov 0x0d(%bx), %dl // bpb_sectors_per_cluster
  mul %dl
  add %eax, (dap_lba) // lba(cluster)
  call eread
  pop %eax
  popl (dap_lba)
  ret

// params
// - %eax: current cluster
// out
// - %ax: next cluster, or zero
get_next_cluster:
  mov fat(,%eax,4), %eax
  and $fat_mask, %eax
  cmp $fat_valid_start, %eax
  jl no_next_cluster
  cmp $fat_valid_end, %eax
  jl has_next_cluster
no_next_cluster:
  xor %eax, %eax
has_next_cluster:
  ret

// params
// - %eax: first cluster
load_file_cluster:
  call load_cluster
  addw $sector_bytes, (dap_dest_addr_offset)
  call get_next_cluster
  test %eax, %eax
  jz load_file_cluster_done
  jmp load_file_cluster
load_file_cluster_done:
  ret

eread:
  mov $2, %cx
eread_loop:
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
  jnc eread_loop
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
loadk_cluster:
  .word 0
kernel_cluster:
  .word 0
ramdisk_cluster:
  .word 0
boot_dir: .ascii "BOOT"
.equ boot_dir_strlen, .-boot_dir
loadk_file: .ascii "LOADK"
.equ loadk_file_strlen, .-loadk_file
// todo: maybe get this from a config instead
kernel_file: .ascii "BEANS"
.equ kernel_file_strlen, .-kernel_file
ramdisk_file: .ascii "RAMDISK"
.equ ramdisk_file_strlen, .-ramdisk_file
.equ mmap, 0x500
.equ fat, 0x8400
.equ loadk, 0x8600
.equ fat_mask, 0x0fffffff
.equ fat_valid_start, 0x00000002
.equ fat_valid_end, 0x0ffffff0
.equ sector_bytes, 0x200

.skip 512-(.-_start), 0
