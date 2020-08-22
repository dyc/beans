PREFIX=/usr/local/cross
AS=$(PREFIX)/bin/i686-elf-as
CC=$(PREFIX)/bin/i686-elf-gcc
ISODIR=iso
OBJDIR=obj

all: myos.iso

$(OBJDIR)/%.o: %.c
	${CC} -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

$(OBJDIR)/%.o: %.s
	${AS} $< -o $@

myos.bin: $(wildcard $(OBJDIR)/*.o)
	${CC} -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

check: myos.bin
	grub-file --is-x86-multiboot myos.bin

myos.iso:
	check
	rm -rf $(ISODIR)
	mkdir -p $(ISODIR)/boot/grub
	cp myos.bin $(ISODIR)/boot/myos.bin
	cp grub.cfg $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/*.o
	rm -rf $(ISODIR)
