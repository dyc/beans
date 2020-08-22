PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc
ISODIR=iso
OBJDIR=obj
SRCDIR=src
BINDIR=bin
CSOURCES=$(wildcard $(SRCDIR)/*.c)
ASMSOURCES=$(wildcard $(SRCDIR)/*.s)
COBJECTS=$(CSOURCES:$(SRCDIR)/.c=$(OBJDIR)/.o)
ASMOBJECTS=$(ASMSOURCES:$(SRCDIR)/.s=$(OBJDIR)/.o)

all: myos.iso

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	${CC} -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(OBJDIR)
	${AS} $< -o $@

$(BINDIR)/myos.bin: $(COBJECTS) $(ASMOBJECTS)
	@mkdir -p $(BINDIR)
	${CC} -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

check: $(BINDIR)/myos.bin
	grub-file --is-x86-multiboot $(BINDIR)/myos.bin

myos.iso: check
	rm -rf $(ISODIR)
	mkdir -p $(ISODIR)/boot/grub
	cp $(BINDIR)/myos.bin $(ISODIR)/boot/myos.bin
	cp grub.cfg $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso $(ISODIR)

qemu: myos.iso
	qemu-system-i386 -cdrom myos.iso

.PHONY: clean
clean:
	rm -rf $(BINDIR)
	rm -rf $(ISODIR)
	rm -rf $(OBJDIR)
	rm myos.iso
