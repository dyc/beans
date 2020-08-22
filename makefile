PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc

ISO_DIR=iso
OBJ_DIR=obj
SRC_DIR=src
BIN_DIR=bin

# order matters here (for linking)
CRTI_OBJ=$(OBJ_DIR)/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
KERNEL_OBJ=$(OBJ_DIR)/kernel.o
BOOT_OBJ=$(OBJ_DIR)/boot.o
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=$(OBJ_DIR)/crtn.o
OBJS:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJ) $(BOOT_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: myos.iso

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	${CC} -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(OBJ_DIR)
	${AS} $< -o $@

$(BIN_DIR)/myos.bin: $(OBJS)
	@mkdir -p $(BIN_DIR)
	${CC} -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

check: $(BIN_DIR)/myos.bin
	grub-file --is-x86-multiboot $(BIN_DIR)/myos.bin

myos.iso: check
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BIN_DIR)/myos.bin $(ISO_DIR)/boot/myos.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso $(ISO_DIR)

qemu: myos.iso
	qemu-system-i386 -cdrom myos.iso

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
	rm -rf $(ISO_DIR)
	rm -rf $(OBJ_DIR)
	rm myos.iso
