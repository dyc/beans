PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc
LD=$(PREFIX)/bin/i686-elf-ld

BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
ISO_DIR=$(BUILD_DIR)/iso
KERNEL_BUILD_DIR=$(BUILD_DIR)/kernel
KERNELASM_BUILD_DIR=$(KERNEL_BUILD_DIR)/asm
KERNELMOD_BUILD_DIR=$(KERNEL_BUILD_DIR)/modules
LIB_BUILD_DIR=$(BUILD_DIR)/lib

SRC_DIR=src
BOOT_SRC_DIR=$(SRC_DIR)/boot
KERNEL_SRC_DIR=$(SRC_DIR)/kernel
KERNELASM_SRC_DIR=$(KERNEL_SRC_DIR)/asm
KERNELMOD_SRC_DIR=$(KERNEL_SRC_DIR)/modules
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
KERNEL_OBJS:=$(filter-out $(wildcard $(KERNELMOD_SRC_DIR/*.c)), $(KERNEL_OBJS))
# asm sources
KERNEL_OBJS:=$(KERNEL_OBJS) $(patsubst %.S,%.o,$(wildcard $(KERNELASM_SRC_DIR)/*.S))
# source path -> build path
KERNEL_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_OBJS))
# link order matters here
KERNEL_OBJS:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(KERNEL_OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

KERNEL_HEADERS = $(wildcard $(SYSROOT_SRC_DIR)/usr/include/kernel/*.h $(SYSROOT_SRC_DIR)/usr/include/kernel/*/*.h)

## modules ##
KERNELMOD_OBJS:=$(patsubst %.c,%.mod,$(wildcard $(KERNELMOD_SRC_DIR)/*.c))
KERNELMOD_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNELMOD_OBJS))
# todo: shot in the dark lol, read up on loading modules...
KMODCFLAGS=-O2 -std=gnu99 -ffreestanding -nostdlib -Wall -Wextra -Werror -Wl,--oformat=binary -fPIE

#### lib ####
# todo: shared libraries
# c sources
LIB_OBJS=$(patsubst %.c,%.o,$(wildcard $(LIB_SRC_DIR)/*.c))
# source path -> build path
LIB_OBJS:=$(patsubst $(LIB_SRC_DIR)/%,$(LIB_BUILD_DIR)/%,$(LIB_OBJS))
LIB_HEADERS = $(wildcard $(SYSROOT_SRC_DIR)/usr/include/sys/*.h $(SYSROOT_SRC_DIR)/usr/include/sys/*/*.h)

all: $(BIN_DIR)/beans.iso

# directories
$(KERNEL_OBJS): | $(KERNEL_BUIlD_DIR) $(KERNELASM_BUILD_DIR)
$(KERNEL_BUILD_DIR):
	@mkdir -p $(KERNEL_BUILD_DIR)
$(KERNELASM_BUILD_DIR):
	@mkdir -p $(KERNELASM_BUILD_DIR)

$(KERNELMOD_OBJS): | $(KERNELMOD_BUILD_DIR)
$(KERNELMOD_BUILD_DIR):
	@mkdir -p $(KERNELMOD_BUILD_DIR)

$(LIB_OBJS): | $(LIB_BUILD_DIR)
$(LIB_BUILD_DIR):
	@mkdir -p $(LIB_BUILD_DIR)

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c $(KERNEL_HEADERS)
	${CC} -c $< -o $@ $(KCFLAGS)

$(KERNELASM_BUILD_DIR)/%.o: $(KERNELASM_SRC_DIR)/%.S
	${AS} $< -o $@

$(KERNELMOD_BUILD_DIR)/%.o: $(KERNELMOD_SRC_DIR)/%.c
	${CC} -c $< -o $@ $(KMODCFLAGS)

# todo: figure out why -Wl isn't working above and remove this
$(KERNELMOD_BUILD_DIR)/%.mod: $(KERNELMOD_BUILD_DIR)/%.o
	${LD} --oformat binary -o $@ $<

$(LIB_BUILD_DIR)/%.o: $(LIB_SRC_DIR)/%.c $(LIB_HEADERS)
	${CC} -c $< -o $@ $(KCFLAGS)

$(BIN_DIR)/beans.bin: $(KERNEL_OBJS) $(LIB_OBJS)
	@mkdir -p $(BIN_DIR)
	${CC} -T $(KERNEL_SRC_DIR)/linker.ld -o $@ $(KCFLAGS) $^ -lgcc

check: $(BIN_DIR)/beans.bin
	grub-file --is-x86-multiboot $(BIN_DIR)/beans.bin

$(BIN_DIR)/beans.iso: check $(KERNELMOD_OBJS)
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	mkdir -p $(ISO_DIR)/modules
	cp $(BIN_DIR)/beans.bin $(ISO_DIR)/boot/beans.bin
	cp $(BOOT_SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	cp $(KERNELMOD_BUILD_DIR)/*.mod $(ISO_DIR)/modules
	grub-mkrescue -o $(BIN_DIR)/beans.iso $(ISO_DIR)

run: $(BIN_DIR)/beans.iso
	qemu-system-i386 -serial stdio -cdrom $(BIN_DIR)/beans.iso

debug: $(BIN_DIR)/beans.iso
	qemu-system-i386 -s -S -serial stdio -cdrom $(BIN_DIR)/beans.iso

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
