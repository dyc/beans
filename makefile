PREFIX:=/usr/local/cross
AS:=$(PREFIX)/bin/i686-elf-as
CC:=$(PREFIX)/bin/i686-elf-gcc
LD:=$(PREFIX)/bin/i686-elf-ld
OBJCOPY:=$(PREFIX)/bin/i686-elf-objcopy

BUILD_DIR:=build
BIN_DIR:=$(BUILD_DIR)/bin
# todo: add back ISO build
ISO_DIR:=$(BUILD_DIR)/iso
BOOT_BUILD_DIR:=$(BUILD_DIR)/boot
KERNEL_BUILD_DIR:=$(BUILD_DIR)/kernel
KERNEL_ASM_BUILD_DIR:=$(KERNEL_BUILD_DIR)/asm
KERNEL_LIB_BUILD_DIR:=$(KERNEL_BUILD_DIR)/lib
KERNEL_MOD_BUILD_DIR:=$(KERNEL_BUILD_DIR)/modules
LIB_BUILD_DIR:=$(BUILD_DIR)/lib
LIBC_BUILD_DIR:=$(BUILD_DIR)/libc

SRC_DIR:=src
BOOT_SRC_DIR:=$(SRC_DIR)/boot
KERNEL_SRC_DIR:=$(SRC_DIR)/kernel
KERNEL_ASM_SRC_DIR:=$(KERNEL_SRC_DIR)/asm
KERNEL_LIB_SRC_DIR:=$(KERNEL_SRC_DIR)/lib
KERNEL_MOD_SRC_DIR:=$(KERNEL_SRC_DIR)/modules
LIB_SRC_DIR:=$(SRC_DIR)/lib
LIBC_SRC_DIR:=$(SRC_DIR)/libc
SYSROOT_SRC_DIR:=$(SRC_DIR)/sysroot
LINKER_SRC_DIR:=$(SRC_DIR)/linker

# keep intermediates on fail
.SECONDARY:

#### bootloader ####
MBR_LINKER_SCRIPT=$(LINKER_SRC_DIR)/mbr.ld
BOOT_LINKER_SCRIPT=$(LINKER_SRC_DIR)/boot.ld

#### kernel ####
KCFLAGS:=-O2 -std=gnu99 -ffreestanding -nostdlib -Wall -Wextra -Werror -Isrc/sysroot/usr/include
CRTI_OBJ:=$(KERNEL_ASM_BUILD_DIR)/crti.o
CRTN_OBJ:=$(KERNEL_ASM_BUILD_DIR)/crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(KCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(KCFLAGS) -print-file-name=crtend.o)
# c sources
KERNEL_OBJS:=$(patsubst %.c,%.o,$(wildcard $(KERNEL_SRC_DIR)/*.c))
# asm sources
KERNEL_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_SRC_DIR)/*.S))
KERNEL_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_SRC_DIR)/*/*.S))
# build modules separately
KERNEL_OBJS:=$(filter-out $(wildcard $(KERNEL_MOD_SRC_DIR/*.c)), $(KERNEL_OBJS))
KERNEL_OBJS:=$(filter-out $(wildcard $(KERNEL_MOD_SRC_DIR/*.S)), $(KERNEL_OBJS))
# source path -> build path
KERNEL_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_OBJS))
# add start objs
KERNEL_OBJS:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)
KERNEL_HEADERS:=$(wildcard $(SYSROOT_SRC_DIR)/usr/include/kernel/*.h $(SYSROOT_SRC_DIR)/usr/include/kernel/*/*.h)
KERNEL_LINKER_SCRIPT:=$(LINKER_SRC_DIR)/kernel.ld
# for convenience
KERNEL_LIB_OBJS:=$(patsubst %.c,%.o,$(wildcard $(KERNEL_LIB_SRC_DIR)/*.c))
KERNEL_LIB_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_LIB_SRC_DIR)/*.S))
KERNEL_LIB_OBJS:=$(filter-out $(wildcard $(KERNEL_ASM_SRC_DIR/*.S)), $(KERNEL_LIB_OBJS))
KERNEL_LIB_OBJS:=$(patsubst $(KERNEL_LIB_SRC_DIR)/%,$(KERNEL_LIB_BUILD_DIR)/%,$(KERNEL_LIB_OBJS))

## modules ##
KERNEL_MOD_OBJS:=$(patsubst %.c,%.o,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.c))
KERNEL_MOD_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.S))
KERNEL_MOD_OBJS:=$(filter-out $(wildcard $(KERNEL_ASM_SRC_DIR/*.S)), $(KERNEL_MOD_OBJS))
KERNEL_MOD_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_MOD_OBJS))
KERNEL_MODS:=$(patsubst %.c,%.ko,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.c))
KERNEL_MODS+=$(patsubst %.S,%.ko,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.S))
KERNEL_MODS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_MODS))
# todo: make these elfs; modules as flat bins for now
KMODCFLAGS:=-ffreestanding -nostartfiles -nostdlib -fPIE -O2 -Isrc/sysroot/usr/include
MODULE_LINKER_SCRIPT:=$(LINKER_SRC_DIR)/module.ld

#### lib ####
# todo: shared libraries
LIB_OBJS:=$(patsubst %.c,%.o,$(wildcard $(LIB_SRC_DIR)/*.c))
LIB_OBJS:=$(patsubst $(LIB_SRC_DIR)/%,$(LIB_BUILD_DIR)/%,$(LIB_OBJS))
LIB_HEADERS:=$(wildcard $(SYSROOT_SRC_DIR)/usr/include/sys/*.h $(SYSROOT_SRC_DIR)/usr/include/sys/*/*.h)

#### libc ####
# todo: shared libraries
LIBC_OBJS:=$(patsubst %.c,%.o,$(wildcard $(LIBC_SRC_DIR)/*.c))
LIBC_OBJS:=$(patsubst $(LIBC_SRC_DIR)/%,$(LIBC_BUILD_DIR)/%,$(LIBC_OBJS))
LIBC_HEADERS:=$(wildcard $(SYSROOT_SRC_DIR)/usr/include/*.h)

all: $(BIN_DIR)/beans.iso

debug: KCFLAGS:=$(filter-out -O2,$(KCFLAGS))
debug: KCFLAGS+=-g
debug: KMODCFLAGS:=$(filter-out -O2,$(KMODCFLAGS))
debug: KMODCFLAGS+=-g
debug: all

# directories
# todo: this is kinda terrible
$(BOOT_BUILD_DIR)/mbr.o: | $(BOOT_BUILD_DIR)
$(BOOT_BUILD_DIR)/boot.o: | $(BOOT_BUILD_DIR)
$(BOOT_BUILD_DIR):
	mkdir -p $@

$(KERNEL_OBJS): | $(KERNEL_BUIlD_DIR) $(KERNEL_ASM_BUILD_DIR)
$(KERNEL_BUILD_DIR):
	mkdir -p $@
$(KERNEL_ASM_BUILD_DIR):
	mkdir -p $@

$(KERNEL_LIB_OBJS): | $(KERNEL_LIB_BUILD_DIR)
$(KERNEL_LIB_BUILD_DIR):
	mkdir -p $@

$(KERNEL_MOD_OBJS): | $(KERNEL_MOD_BUILD_DIR)
$(KERNEL_MODS): | $(KERNEL_MOD_BUILD_DIR)
$(KERNEL_MOD_BUILD_DIR):
	mkdir -p $@

$(LIB_OBJS): | $(LIB_BUILD_DIR)
$(LIB_BUILD_DIR):
	mkdir -p $@

$(LIBC_OBJS): | $(LIBC_BUILD_DIR)
$(LIBC_BUILD_DIR):
	mkdir -p $@

$(BIN_DIR)/mbr: | $(BIN_DIR)
$(BIN_DIR)/boot: | $(BIN_DIR)
$(BIN_DIR)/beans: | $(BIN_DIR)
$(BIN_DIR):
	mkdir -p $@

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.S
	$(AS) $< -o $@

$(KERNEL_ASM_BUILD_DIR)/%.o: $(KERNEL_ASM_SRC_DIR)/%.S
	$(AS) $< -o $@

$(KERNEL_LIB_BUILD_DIR)/%.o: $(KERNEL_LIB_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(KERNEL_LIB_BUILD_DIR)/%.o: $(KERNEL_LIB_SRC_DIR)/%.S
	$(error here)
	$(AS) $< -o $@

$(KERNEL_MOD_BUILD_DIR)/%.o: $(KERNEL_MOD_SRC_DIR)/%.c
	$(CC) $(KMODCFLAGS) -c $< -o $@

# todo: this is a bit of a mess rn lol
$(KERNEL_MOD_BUILD_DIR)/%.ko: $(KERNEL_MOD_BUILD_DIR)/%.o $(KERNEL_LIB_OBJS)
	$(CC) -T $(MODULE_LINKER_SCRIPT) $(KMODCFLAGS) $< -o $@ $(KERNEL_LIB_OBJS)

$(LIB_BUILD_DIR)/%.o: $(LIB_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(LIBC_BUILD_DIR)/%.o: $(LIBC_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(BIN_DIR)/beans: $(KERNEL_OBJS) $(KERNEL_LIB_OBJS) $(LIB_OBJS)
	$(CC) -T $(KERNEL_LINKER_SCRIPT) -o $@ $(KCFLAGS) $^ -lgcc

$(BOOT_BUILD_DIR)/%.o: $(BOOT_SRC_DIR)/%.S
	$(AS) $< -o $@

$(BIN_DIR)/%: $(BOOT_BUILD_DIR)/%.o
	$(LD) -T $(LINKER_SRC_DIR)/$(@F).ld -o $@ $^

# todo: fix this once we successfully make a bootable image
# some useful debugging tools:
# - hdiutil imageinfo and hdiutil pmap
# - diskutil info, list
# - file
# perhaps try mounting the created image to add files? (need to figure out how to
# copy bootloader and kernel to image...)
$(BIN_DIR)/beans.img: $(BIN_DIR)/mbr $(BIN_DIR)/boot $(BIN_DIR)/beans $(KERNEL_MODS)
	# 80mb of 512b blocks
	dd if=/dev/zero of=$(BIN_DIR)/blank.img count=163840 bs=512
	# todo: script this
	# make partition table
	# this seems to be preferable to something like diskutil's partitionDisk,
	# but in case we end up going that route this generates something reasonable
	# (seems like there are a couple extra apple_free sections inserted for some
	# reason...and the mbr section is only 1 byte...):
	# diskutil partitionDisk /dev/diskN 2 MBR FREE 2048B FAT32 BEANSFS R
	#
	# and then resize the blocks:
	# diskutil unmountDisk /dev/diskN
	# hdiutil attach -nomount /dev/diskN
	# newfs_msdos -v BEANSFS -b 512 /dev/diskNs1
	fdisk -e $(BIN_DIR)/blank.img
	# cut out mbr, rest is fs
	dd if=$(BIN_DIR)/blank.img of=$(BIN_DIR)/fs.img bs=512 skip=2047
	# create fat32 on fs.img
	tempdevice=$(shell hdiutil attach -nomount $(BIN_DIR)/fs.img); \
		echo "$${tempdevice}" | xargs newfs_msdos -F 32; \
		echo "$${tempdevice}" | xargs hdiutil detach
	# noop but do it anyways
	dd if=$(BIN_DIR)/mbr of=$(BIN_DIR)/mbr.img bs=512 count=1
	# fill to 1mb
	dd if=/dev/zero bs=1 seek=512 count=1047552 >> $(BIN_DIR)/mbr.img
	# make our final disk
	# rn, generates something semisensical but likely corrupt? i see
	# reasonable partition info (active first partition, correct start/end CHS)
	# but i'm guessing that's just stuff inserted by fdisk and not
	# read from our mbr table? idk
	cat $(BIN_DIR)/mbr.img $(BIN_DIR)/fs.img > $(BIN_DIR)/beans.img
	file $(BIN_DIR)/beans.img

.PHONY: run
run: $(BIN_DIR)/beans.iso
	qemu-system-i386 -serial stdio -disk format=raw,file=$(BIN_DIR)/beans.img

.PHONY: gdb
gdb: debug
	$(OBJCOPY) --only-keep-debug $(BIN_DIR)/beans $(BIN_DIR)/beans.sym
	$(OBJCOPY) --strip-debug $(BIN_DIR)/beans
	qemu-system-i386 -s -S -serial stdio -disk format=raw,file=$(BIN_DIR)/beans.img

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
