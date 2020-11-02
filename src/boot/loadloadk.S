// loads /boot/loadk, /boot/kernel, /boot/initrd,
// in that order, to $LOAD_BASE
// clobbery :'-)
// org 0x8000 (from linker script)
.code16
_start:
  popw (e820_count)
  popw (drive_number)
  pop %bx // bpb_start
  xor %eax, %eax
  mov %ax, %ds
  mov %ax, %es

// assume everything we need is in the first fat
load_fat:
  movw $FAT_START, (dap_dest_addr_offset)
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
  addw $SECTOR_BYTES, (dap_dest_addr_offset)

load_root_cluster:
  mov 0x2c(%bx), %eax // root cluster num
  call load_cluster
find_boot_dir:
  mov (dap_dest_addr_offset), %si
  mov $BOOT_DIR, %di
find_boot_dir_loop:
  mov $BOOT_DIR_STRLEN, %cl
  add $0x20, %si
  call cmpstr
  jne find_boot_dir_loop
  call load_cluster_from_dirent

  // find each of loadk, kernel and initrd
  mov $3, %dl
  mov (dap_dest_addr_offset), %si
check_loadk:
  mov $LOADK_FILE, %di
  mov $LOADK_FILE_STRLEN, %cl
  call cmpstr
  jne check_kernel
  call get_cluster_info
  mov %ax, (loadk_cluster)
  jmp maybe_check_next
check_kernel:
  mov $KERNEL_FILE, %di
  mov $KERNEL_FILE_STRLEN, %cl
  call cmpstr
  jne check_initrd
  call get_cluster_info
  mov %ax, (kernel_cluster)
  jmp maybe_check_next
check_initrd:
  mov $INITRD_FILE, %di
  mov $INITRD_FILE_STRLEN, %cl
  call cmpstr
  jne check_next
  call get_cluster_info
  mov %ax, (initrd_cluster)
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
  movw $0x0000, (dap_dest_addr_offset)
  movw $LOAD_SEGMENT, (dap_dest_addr_segment)
load_loadk:
  mov (loadk_cluster), %ax
  call load_file_cluster
load_kernel:
  mov (dap_dest_addr_offset), %ax
  mov %ax, (kernel_addr)
  addl $LOAD_BASE, (kernel_addr)
  mov (kernel_cluster), %ax
  call load_file_cluster
load_initrd:
  mov (dap_dest_addr_offset), %ax
  mov %ax, (initrd_addr)
  addl $LOAD_BASE, (initrd_addr)
  mov (initrd_cluster), %ax
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
  ljmp $0x08, $trampoline
.code32
trampoline:
  pushl (initrd_addr)
  pushl (kernel_addr)
  pushl $MMAP_START
  xor %eax, %eax
  mov (e820_count), %al
  push %eax
  pushl $hang
  mov $LOAD_BASE, %eax
  jmp *%eax

.code16
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
  mov FAT_START(,%eax,4), %eax
  and $FAT_MASK, %eax
  cmp $FAT_VALID_START, %eax
  jl no_next_cluster
  cmp $FAT_VALID_END, %eax
  jl has_next_cluster
no_next_cluster:
  xor %eax, %eax
has_next_cluster:
  ret

// params
// - %eax: first cluster
load_file_cluster:
  call load_cluster
  addw $SECTOR_BYTES, (dap_dest_addr_offset)
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
kernel_addr:
  .word 0
  .word 0
initrd_cluster:
  .word 0
initrd_addr:
  .word 0
  .word 0
BOOT_DIR: .ascii "BOOT"
.set BOOT_DIR_STRLEN, .-BOOT_DIR
LOADK_FILE: .ascii "LOADK"
.set LOADK_FILE_STRLEN, .-LOADK_FILE
KERNEL_FILE: .ascii "BEANS"
.set KERNEL_FILE_STRLEN, .-KERNEL_FILE
INITRD_FILE: .ascii "INITRD"
.set INITRD_FILE_STRLEN, .-INITRD_FILE
.set MMAP_START, 0x500
.set FAT_START, 0x8200
.set LOAD_SEGMENT, 0x1000
.set LOAD_BASE, 0x10000
.set FAT_MASK, 0x0fffffff
.set FAT_VALID_START, 0x00000002
.set FAT_VALID_END, 0x0ffffff0
.set SECTOR_BYTES, 0x200

.skip 512-(.-_start), 0
