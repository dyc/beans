PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc
CFLAGS=-O2 -Wall -Wextra

BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
BIN_DIR=$(BUILD_DIR)/bin
ISO_DIR=$(BUILD_DIR)/iso
SRC_DIR=src

# order matters here (for linking)
CRTI_OBJ=$(OBJ_DIR)/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
KERNEL_OBJ=$(OBJ_DIR)/kernel.o
# effectively our crt0.o
BOOT_OBJ=$(OBJ_DIR)/boot.o
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=$(OBJ_DIR)/crtn.o
OBJS:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJ) $(BOOT_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: $(BIN_DIR)/bbos.iso

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	${CC} -c $< -o $@ -std=gnu99 -ffreestanding $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(OBJ_DIR)
	${AS} $< -o $@

$(BIN_DIR)/bbos.bin: $(OBJS)
	@mkdir -p $(BIN_DIR)
	${CC} -T linker.ld -o $@ -ffreestanding -nostdlib $(CFLAGS) $^ -lgcc

check: $(BIN_DIR)/bbos.bin
	grub-file --is-x86-multiboot $(BIN_DIR)/bbos.bin

$(BIN_DIR)/bbos.iso: check
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BIN_DIR)/bbos.bin $(ISO_DIR)/boot/bbos.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BIN_DIR)/bbos.iso $(ISO_DIR)

qemu: $(BIN_DIR)/bbos.iso
	qemu-system-i386 -cdrom $(BIN_DIR)/bbos.iso

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
