// loads /boot/loadk, /boot/kernel, /boot/ramdisk
// into memory starting at $0x8400 (in that order)
// clobbery :'-)
// org 0x8200 (from linker script)
.code16
_start:
  popw (e820_count)
  popw (drive_number)
  popw %bx // bpb_start
  xor %eax, %eax
  // clear ds, es for cmpstr
  movw %ax, %ds
  movw %ax, %es

// assume everything we need is in the first fat
load_fat:
  movw $0x8400, (dap_dest_addr_offset)
  // account for partition LBA offset
  // could also use pte's lba: 8(pte)
  movl 0x1c(%bx), %eax // bpb_hidden_sectors
  // within partition, fat starts here
  addw 0x0e(%bx), %ax // bpb_reserved_sectors (typically 32)
  mov %eax, (dap_lba) // fat lba
  call eread
  xor %eax, %eax
  movb 0x10(%bx), %al // bpb_fats
  mull 0x24(%bx) // ebpb_sectors_per_fat
  add %eax, (dap_lba) // root cluster lba
  // loads will go right after fat
  addw $0x200, (dap_dest_addr_offset)

load_root_cluster:
  movl 0x2c(%bx), %eax // root cluster num
  call load_cluster
find_boot_dir:
  movw (dap_dest_addr_offset), %si
  mov $boot_dir, %di
find_boot_dir_loop:
  movb $boot_dir_strlen, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
  call load_cluster_from_dirent

  // find each of loadk, kernel and ramdisk
  mov $3, %dl
  movw (dap_dest_addr_offset), %si
check_loadk:
  mov $loadk_file, %di
  movb $loadk_file_strlen, %cl
  call cmpstr
  jne check_kernel
  call get_cluster_info
  movw %ax, (loadk_cluster)
  movb %cl, (loadk_size)
  jmp maybe_check_next
check_kernel:
  mov $kernel_file, %di
  movb $kernel_file_strlen, %cl
  call cmpstr
  jne check_ramdisk
  call get_cluster_info
  movw %ax, (kernel_cluster)
  movb %cl, (kernel_size)
  jmp maybe_check_next
check_ramdisk:
  mov $ramdisk_file, %di
  movb $ramdisk_file_strlen, %cl
  call cmpstr
  jne check_next
  call get_cluster_info
  movw %ax, (ramdisk_cluster)
  movb %cl, (ramdisk_size)
  jmp maybe_check_next

// out
// - %ax: cluster num
// - %cx: size in sectors
get_cluster_info:
  pushw %dx
  xor %dx, %dx
  movw 0x1c(%si), %ax // size in bytes
  divw 0x0b(%bx) // bytes per sector
  cmp $0, %dx
  je no_remainder
  addw $1, %ax
no_remainder:
  mov %ax, %cx
  movw 0x14(%si), %ax
  shl $16, %eax
  movw 0x1a(%si), %ax
  popw %dx
  ret
maybe_check_next:
  dec %dl
  jz load_start
check_next:
  add $0x20, %si
  jmp check_loadk
load_start:
  movw $0x8600, (dap_dest_addr_offset)
load_loadk:
  movw (loadk_cluster), %ax
  movb (loadk_size), %cl
  // todo: consult fat, read all clusters
  movb %cl, (dap_blocks)
  call load_cluster
  shlw $9, %cx
  addw %cx, (dap_dest_addr_offset)
load_kernel:
  movw (kernel_cluster), %ax
  movb (kernel_size), %cl
  movb %cl, (dap_blocks)
  call load_cluster
  shlw $9, %cx
  addw %cx, (dap_dest_addr_offset)
load_ramdisk:
  movw (ramdisk_cluster), %ax
  movb (ramdisk_size), %cl
  movb %cl, (dap_blocks)
  call load_cluster

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
  ljmp $0x08, $0x8600

// params
// - %si: dirent
// out
// - %eax: cluster number
load_cluster_from_dirent:
  movw 0x14(%si), %ax
  shl $16, %eax
  movw 0x1a(%si), %ax
// params
// - %eax: cluster number
// out
// - %eax: lba(cluster), relative to first cluster
load_cluster:
  pushl (dap_lba)
  subl $2, %eax // root cluster offset
  movb 0x0d(%bx), %dl // bpb_sectors_per_cluster
  mulb %dl
  addl %eax, (dap_lba) // lba(cluster)
  call eread
  popl (dap_lba)
  ret

eread:
  pushw %cx
  movw $2, %cx
eread_loop:
  leaw dap, %si
  movb (drive_number), %dl
  movb $0x42, %ah
  int $0x13
  jnc eread_done
  decw %cx
  jz hang
  // reset and try again
  xorb %ah, %ah
  int $0x13
  jnc eread_loop
  jmp hang
eread_done:
  popw %cx
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

.skip 512-(.-_start), 0
