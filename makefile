PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc
CFLAGS=-O2 -Wall -Wextra -Werror

BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
ISO_DIR=$(BUILD_DIR)/iso
KERNEL_BUILD_DIR=$(BUILD_DIR)/kernel

SRC_DIR=src
BOOT_SRC_DIR=$(SRC_DIR)/boot
KERNEL_SRC_DIR=$(SRC_DIR)/kernel

CRTI_OBJ=$(KERNEL_BUILD_DIR)/crti.o
CRTBEGIN_OBJ=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
KERNEL_OBJ=$(KERNEL_BUILD_DIR)/main.o
# effectively crt0.o
KERNEL_BOOT_OBJ=$(KERNEL_BUILD_DIR)/boot.o
CRTEND_OBJ=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=$(KERNEL_BUILD_DIR)/crtn.o
# link order matters here
OBJS=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJ) $(KERNEL_BOOT_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: $(BIN_DIR)/bbos.iso

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@mkdir -p $(KERNEL_BUILD_DIR)
	${CC} -c $< -o $@ -std=gnu99 -ffreestanding $(CFLAGS)

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.S
	@mkdir -p $(KERNEL_BUILD_DIR)
	${AS} $< -o $@

$(BIN_DIR)/bbos.bin: $(OBJS)
	@mkdir -p $(BIN_DIR)
	${CC} -T $(KERNEL_SRC_DIR)/linker.ld -o $@ -ffreestanding -nostdlib $(CFLAGS) $^ -lgcc

check: $(BIN_DIR)/bbos.bin
	grub-file --is-x86-multiboot $(BIN_DIR)/bbos.bin

$(BIN_DIR)/bbos.iso: check
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BIN_DIR)/bbos.bin $(ISO_DIR)/boot/bbos.bin
	cp $(BOOT_SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BIN_DIR)/bbos.iso $(ISO_DIR)

qemu: $(BIN_DIR)/bbos.iso
	qemu-system-i386 -cdrom $(BIN_DIR)/bbos.iso

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
