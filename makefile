PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc

BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
ISO_DIR=$(BUILD_DIR)/iso
KERNEL_BUILD_DIR=$(BUILD_DIR)/kernel
KERNELASM_BUILD_DIR=$(KERNEL_BUILD_DIR)/asm
LIB_BUILD_DIR=$(BUILD_DIR)/lib

SRC_DIR=src
BOOT_SRC_DIR=$(SRC_DIR)/boot
KERNEL_SRC_DIR=$(SRC_DIR)/kernel
KERNELASM_SRC_DIR=$(KERNEL_SRC_DIR)/asm
LIB_SRC_DIR=$(SRC_DIR)/lib
SYSROOT_SRC_DIR=$(SRC_DIR)/sysroot

#### kernel ####
KCFLAGS=-O2 -std=gnu99 -ffreestanding -nostdlib -Wall -Wextra -Werror -Isrc/sysroot/usr/include
CRTI_OBJ=$(KERNELASM_BUILD_DIR)/crti.o
CRTN_OBJ=$(KERNELASM_BUILD_DIR)/crtn.o
CRTBEGIN_OBJ=$(shell $(CC) $(KCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ=$(shell $(CC) $(KCFLAGS) -print-file-name=crtend.o)
# c sources
KERNEL_OBJS=$(patsubst %.c,%.o,$(wildcard $(KERNEL_SRC_DIR)/*.c))
# asm sources
KERNEL_OBJS:=$(KERNEL_OBJS) $(patsubst %.S,%.o,$(wildcard $(KERNELASM_SRC_DIR)/*.S))
# source path -> build path
KERNEL_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_OBJS))
# link order matters here
KERNEL_OBJS:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

KERNEL_HEADERS = $(wildcard $(SYSROOT_SRC_DIR)/usr/include/kernel/*.h $(SYSROOT_SRC_DIR)/usr/include/kernel/*/*.h)

#### lib ####
# TODO: shared libraries
# c sources
LIB_OBJS=$(patsubst %.c,%.o,$(wildcard $(LIB_SRC_DIR)/*.c))
# source path -> build path
LIB_OBJS:=$(patsubst $(LIB_SRC_DIR)/%,$(LIB_BUILD_DIR)/%,$(LIB_OBJS))
LIB_HEADERS = $(wildcard $(SYSROOT_SRC_DIR)/usr/include/sys/*.h $(SYSROOT_SRC_DIR)/usr/include/sys/*/*.h)

all: $(BIN_DIR)/howdy.iso

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c $(KERNEL_HEADERS)
	@mkdir -p $(KERNEL_BUILD_DIR)
	${CC} -c $< -o $@ $(KCFLAGS)

$(KERNELASM_BUILD_DIR)/%.o: $(KERNELASM_SRC_DIR)/%.S
	@mkdir -p $(KERNELASM_BUILD_DIR)
	${AS} $< -o $@

$(LIB_BUILD_DIR)/%.o: $(LIB_SRC_DIR)/%.c $(LIB_HEADERS)
	@mkdir -p $(LIB_BUILD_DIR)
	${CC} -c $< -o $@ $(KCFLAGS)

$(BIN_DIR)/howdy.bin: $(KERNEL_OBJS) $(LIB_OBJS)
	@mkdir -p $(BIN_DIR)
	${CC} -T $(KERNEL_SRC_DIR)/linker.ld -o $@ $(KCFLAGS) $^ -lgcc

check: $(BIN_DIR)/howdy.bin
	grub-file --is-x86-multiboot $(BIN_DIR)/howdy.bin

$(BIN_DIR)/howdy.iso: check
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BIN_DIR)/howdy.bin $(ISO_DIR)/boot/howdy.bin
	cp $(BOOT_SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BIN_DIR)/howdy.iso $(ISO_DIR)

run: $(BIN_DIR)/howdy.iso
	qemu-system-i386 -serial stdio -cdrom $(BIN_DIR)/howdy.iso

debug: $(BIN_DIR)/howdy.iso
	qemu-system-i386 -s -S -serial stdio -cdrom $(BIN_DIR)/howdy.iso

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
