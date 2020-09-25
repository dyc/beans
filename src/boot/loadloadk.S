// loads /boot/loadk, /boot/kernel, /boot/ramdisk
// into memory starting at $0x8400 (in that order)
// clobbery :'-)
// org 0x8200 (from linker script)
.code16
_start:
  pop (e820_count)
  pop (drive_number)
  popw %bx // bpb_start
  movw $0x8400, (dap_dest_addr_offset)
  xor %ax, %ax
  // clear ds, es for cmpstr
  mov %ax, %ds
  mov %ax, %es
  // calculate first cluster lba
  // and save in dap_lba
  movb 0x10(%bx), %al // bpb_fats
  mull 0x24(%bx) // ebpb_sectors_per_fat
  addw 0x0e(%bx), %ax // bpb_reserved_sectors (typically 32)
  // account for partition LBA offset
  // i.e. bpb_hidden_sectors
  // could also use pte's lba: 8(pte)
  addl 0x1c(%bx), %eax
  mov %eax, (dap_lba)

load_root_cluster:
  movl 0x2c(%bx), %eax // root cluster num
  call load_cluster

find_boot_dir:
  mov (dap_dest_addr_offset), %si
  mov $boot_dir, %di
find_boot_dir_loop:
  movb $boot_dir_strlen, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
  call load_cluster_from_dirent

  // find each of loadk, kernel and ramdisk
  mov $3, %dh
  mov (dap_dest_addr_offset), %si
check_loadk:
  mov $loadk_file, %di
  movb $loadk_file_strlen, %cl
  call cmpstr
  jne check_kernel
  call found_file_common
  jmp found_loadk
check_kernel:
  mov $kernel_file, %di
  movb $kernel_file_strlen, %cl
  call cmpstr
  jne check_ramdisk
  call found_file_common
  jmp found_kernel
check_ramdisk:
  mov $ramdisk_file, %di
  movb $ramdisk_file_strlen, %cl
  call cmpstr
  jne find_next
  call found_file_common
  jmp found_ramdisk
found_file_common:
  movw 0x14(%si), %ax
  shl $16, %eax
  movw 0x1a(%si), %ax
  // todo: this
  // movl 0x1c(%si), %ecx
  // shr $9, %ecx
  movb $1, %cl // size in sectors
  ret
found_loadk:
  movw %ax, (loadk_cluster)
  movb %cl, (loadk_size)
  jmp maybe_find_next
found_kernel:
  movw %ax, (kernel_cluster)
  movb %cl, (kernel_size)
  jmp maybe_find_next
found_ramdisk:
  movw %ax, (ramdisk_cluster)
  movb %cl, (ramdisk_size)
  jmp maybe_find_next
maybe_find_next:
  dec %dh
  jz load_start
find_next:
  add $0x20, %si
  jmp check_loadk
load_start:
  // todo: use size in load and to calculate dest addr
  movw $0x8400, (dap_dest_addr_offset)
load_loadk:
  movw (loadk_cluster), %ax
  // movb (loadk_size), %al
  // movb %al, (dap_blocks)
  call load_cluster
  addw $0x200, (dap_dest_addr_offset)
load_kernel:
  movw (kernel_cluster), %ax
  // movb (kernel_size), %al
  // movb %al, (dap_blocks)
  call load_cluster
  addw $0x200, (dap_dest_addr_offset)
load_ramdisk:
  movw (ramdisk_cluster), %ax
  // movb (ramdisk_size), %al
  // movb %al, (dap_blocks)
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
  ljmp $0x08, $0x8400

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
  call eread_start
  popl (dap_lba)
  ret

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
