PREFIX:=/usr/local/cross
AS:=$(PREFIX)/bin/i686-elf-as
CC:=$(PREFIX)/bin/i686-elf-gcc
LD:=$(PREFIX)/bin/i686-elf-ld
OBJCOPY:=$(PREFIX)/bin/i686-elf-objcopy
HOSTCC:=/usr/bin/gcc

BUILD_DIR:=build
BIN_DIR:=$(BUILD_DIR)/bin
# todo: add back ISO build
ISO_DIR:=$(BUILD_DIR)/iso
BOOT_BUILD_DIR:=$(BUILD_DIR)/boot
KERNEL_BUILD_DIR:=$(BUILD_DIR)/kernel
KERNEL_ASM_BUILD_DIR:=$(KERNEL_BUILD_DIR)/asm
KERNEL_LIB_BUILD_DIR:=$(KERNEL_BUILD_DIR)/lib
KERNEL_MOD_BUILD_DIR:=$(KERNEL_BUILD_DIR)/modules
KERNEL_MISC_BUILD_DIRS:=$(KERNEL_BUILD_DIR)/fs
LIB_BUILD_DIR:=$(BUILD_DIR)/lib
LIBC_BUILD_DIR:=$(BUILD_DIR)/libc
SYMBOLS_BUILD_DIR:=$(BUILD_DIR)/sym
HOST_BUILD_DIR:=$(BUILD_DIR)/host
INITRD_BUILD_DIR:=$(BUILD_DIR)/initrd

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
HOST_DIR:=host
HOST_SCRIPTS_DIR:=$(HOST_DIR)/scripts

# keep intermediates on fail
.SECONDARY:

#### bootloader ####
BCFLAGS:=-Os -std=gnu99 -ffreestanding -nostdlib -Wall -Wextra -Werror
BOOT_OBJS:=$(patsubst %.S,%.o,$(wildcard $(BOOT_SRC_DIR)/*.S))
BOOT_OBJS+=$(patsubst %.c,%.o,$(wildcard $(BOOT_SRC_DIR)/*.c))
BOOT_OBJS:=$(patsubst $(BOOT_SRC_DIR)/%,$(BOOT_BUILD_DIR)/%,$(BOOT_OBJS))
BOOT_BINS:=$(patsubst %.o,%,$(BOOT_OBJS))
BOOT_ELFS:=$(BOOT_BINS)
BOOT_BINS:=$(patsubst $(BOOT_BUILD_DIR)/%,$(BIN_DIR)/%,$(BOOT_BINS))

#### kernel ####
KCFLAGS:=-O2 -std=gnu99 -ffreestanding -nostdlib -Wall -Wextra -Werror -Isrc/sysroot/usr/include
CRTI_OBJ:=$(KERNEL_ASM_BUILD_DIR)/crti.o
CRTN_OBJ:=$(KERNEL_ASM_BUILD_DIR)/crtn.o
CRTBEGIN_OBJ:=$(shell $(CC) $(KCFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(KCFLAGS) -print-file-name=crtend.o)
# c sources
KERNEL_OBJS:=$(patsubst %.c,%.o,$(wildcard $(KERNEL_SRC_DIR)/*.c))
KERNEL_OBJS+=$(patsubst %.c,%.o,$(wildcard $(KERNEL_SRC_DIR)/*/*.c))
# asm sources
KERNEL_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_SRC_DIR)/*.S))
KERNEL_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_SRC_DIR)/*/*.S))
# build lib and modules separately
KERNEL_OBJS:=$(filter-out $(wildcard $(LIB_SRC_DIR/*.c)), $(KERNEL_OBJS))
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
# todo: make these elfs; modules as flat bins for now
KMODCFLAGS:=-ffreestanding -nostartfiles -nostdlib -fPIE -O2 -Isrc/sysroot/usr/include
KERNEL_MOD_OBJS:=$(patsubst %.c,%.o,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.c))
KERNEL_MOD_OBJS+=$(patsubst %.S,%.o,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.S))
KERNEL_MOD_OBJS:=$(filter-out $(wildcard $(KERNEL_ASM_SRC_DIR/*.S)), $(KERNEL_MOD_OBJS))
KERNEL_MOD_OBJS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_MOD_OBJS))
KERNEL_MODS:=$(patsubst %.c,%.ko,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.c))
KERNEL_MODS+=$(patsubst %.S,%.ko,$(wildcard $(KERNEL_MOD_SRC_DIR)/*.S))
KERNEL_MODS:=$(patsubst $(KERNEL_SRC_DIR)/%,$(KERNEL_BUILD_DIR)/%,$(KERNEL_MODS))
MODULE_LINKER_SCRIPT:=$(LINKER_SRC_DIR)/module.ld

## initrd ##
INITRD_SRC_OBJS:=$(KERNEL_MOD_BUILD_DIR)/ata.ko
INITRD_OBJS:=$(notdir $(INITRD_SRC_OBJS))
INITRD_OBJS:=$(patsubst %,$(INITRD_BUILD_DIR)/%,$(INITRD_OBJS))

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

#### host ####
HOSTCFLAGS:=-O3 -std=gnu99 -Wall -Wextra -Werror 
HOST_BINS:=$(patsubst %.c,%,$(wildcard $(HOST_DIR)/*.c))
HOST_BINS:=$(patsubst $(HOST_DIR)/%,$(HOST_BUILD_DIR)/%,$(HOST_BINS))

all: $(BIN_DIR)/beans.img

debug: KCFLAGS:=$(filter-out -O2,$(KCFLAGS))
debug: KCFLAGS+=-g
debug: KMODCFLAGS:=$(filter-out -O2,$(KMODCFLAGS))
debug: KMODCFLAGS+=-g
debug: all

force: BCFLAGS:=$(filter-out -Werror,$(BCFLAGS))
force: KCFLAGS:=$(filter-out -Werror,$(KCFLAGS))
force: KMODCFLAGS:=$(filter-out -Werror,$(KMODCFLAGS))
force: all

# directories
$(BOOT_OBJS): | $(BOOT_BUILD_DIR)
$(BOOT_BUILD_DIR):
	mkdir -p $@

$(BOOT_BINS): | $(BIN_DIR)
$(BIN_DIR)/beans.img: | $(BIN_DIR)
$(BIN_DIR)/initrd.img: | $(BIN_DIR)
$(BIN_DIR):
	mkdir -p $@

$(KERNEL_OBJS): | $(KERNEL_BUIlD_DIR) $(KERNEL_ASM_BUILD_DIR) $(KERNEL_MISC_BUILD_DIRS)
$(KERNEL_BUILD_DIR):
	mkdir -p $@
$(KERNEL_ASM_BUILD_DIR):
	mkdir -p $@
$(KERNEL_MISC_BUILD_DIRS):
	$(foreach d, $(KERNEL_MISC_BUILD_DIRS), mkdir -p $(d);)

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

initrd_files: | $(INITRD_BUILD_DIR)
$(INITRD_BUILD_DIR):
	mkdir -p $@

$(HOST_BINS): | $(HOST_BUILD_DIR)
$(HOST_BUILD_DIR):
	mkdir -p $@

$(SYMBOLS_BUILD_DIR):
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
	$(AS) $< -o $@

$(KERNEL_MOD_BUILD_DIR)/%.o: $(KERNEL_MOD_SRC_DIR)/%.c
	$(CC) $(KMODCFLAGS) -c $< -o $@

# todo: this is a bit of a mess
$(KERNEL_MOD_BUILD_DIR)/%.ko: $(KERNEL_MOD_BUILD_DIR)/%.o $(KERNEL_LIB_OBJS)
	$(CC) -T $(MODULE_LINKER_SCRIPT) $(KMODCFLAGS) $< -o $@ $(KERNEL_LIB_OBJS)

$(LIB_BUILD_DIR)/%.o: $(LIB_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(LIBC_BUILD_DIR)/%.o: $(LIBC_SRC_DIR)/%.c
	$(CC) $(KCFLAGS) -c $< -o $@

$(BIN_DIR)/beans: $(KERNEL_OBJS) $(KERNEL_LIB_OBJS) $(LIB_OBJS)
	$(CC) -T $(KERNEL_LINKER_SCRIPT) -o $@ $(KCFLAGS) $^

$(BOOT_BUILD_DIR)/%.o: $(BOOT_SRC_DIR)/%.c
	$(CC) $(BCFLAGS) -c $< -o $@

$(BOOT_BUILD_DIR)/%.o: $(BOOT_SRC_DIR)/%.S
	$(AS) $< -o $@

$(BIN_DIR)/%: $(BOOT_BUILD_DIR)/%.o
	$(LD) -T $(LINKER_SRC_DIR)/$(@F).ld -o $@ $^

$(BOOT_BUILD_DIR)/%: $(BOOT_BUILD_DIR)/%.o
	$(LD) -o $@ $^

.PHONY: initrd_files
initrd_files: $(INITRD_SRC_OBJS)
	for f in $(INITRD_SRC_OBJS); do cp -f $$f $(INITRD_BUILD_DIR); done

$(BIN_DIR)/initrd.img: $(HOST_BUILD_DIR)/mkinitrd initrd_files
	$(HOST_BUILD_DIR)/mkinitrd $(BIN_DIR)/initrd.img $(INITRD_BUILD_DIR)

$(BIN_DIR)/beans.img: $(BOOT_BINS) $(BIN_DIR)/beans $(BIN_DIR)/initrd.img $(KERNEL_MODS) $(SYSROOT_SRC_DIR)
	$(HOST_SCRIPTS_DIR)/mkimg $(BIN_DIR) $(SYSROOT_SRC_DIR)

# todo: make this cross compatible
$(HOST_BUILD_DIR)/%: $(HOST_DIR)/%.c
	$(HOSTCC) $(HOSTCFLAGS) $< -o $@

.PHONY: run
run: $(BIN_DIR)/beans.img
	qemu-system-i386 -serial stdio -drive format=raw,file=$(BIN_DIR)/beans.img

# todo: look into using elf symbols with flat binary (e.g. for loadk)
.PHONY: gdb
gdb: debug $(BOOT_BUILD_DIR)/loadk | $(SYMBOLS_BUILD_DIR)
	$(OBJCOPY) --only-keep-debug $(BIN_DIR)/beans $(SYMBOLS_BUILD_DIR)/beans.debug
	$(OBJCOPY) --strip-debug $(BIN_DIR)/beans
	$(OBJCOPY) --only-keep-debug $(BOOT_BUILD_DIR)/loadk $(SYMBOLS_BUILD_DIR)/loadk.debug
	qemu-system-i386 -s -S -serial stdio -drive format=raw,file=$(BIN_DIR)/beans.img

.PHONY: lint
lint:
	clang-format -i $(shell find . -name *.c)
	clang-format -i $(shell find . -name *.h)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
