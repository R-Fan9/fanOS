AS = nasm
AFLAGS = -f bin -i src/boot/include

CC = gcc
CFLAGS = -m32 -fno-PIC -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
			-nostartfiles -nodefaultlibs -ffreestanding -Wall -Wextra -Werror -g -c

LD = ld
LFLAGS = -m elf_i386

QEMU = qemu-system-i386 
QFLAGS = -fda 

boot_bin_files = build/Boot.bin build/KRNLDR.SYS build/KRNL.SYS

build/kernel.o: src/kernel.c
	$(CC) $(CFLAGS) $< -o $@

build/stage3.o: src/boot/stage3.asm
	$(AS) -f elf32 $< -o $@

build/KRNL.SYS: build/stage3.o build/kernel.o
	$(LD) $(LFLAGS) -o $@ -T target/link.ld $^

build/KRNLDR.SYS: src/boot/stage2.asm
	$(AS) $(AFLAGS) $< -o $@

build/Boot.bin: src/boot/boot.asm
	$(AS) $(AFLAGS) $< -o $@

bin/OS.bin: $(boot_bin_files)
	dd if=/dev/zero of=bin/OS.bin bs=512   count=2880           # floppy is 2880 sectors of 512 bytes
	dd if=build/Boot.bin of=bin/OS.bin seek=0 count=1 conv=notrunc
	mcopy -i bin/OS.bin build/KRNL.SYS \:\:KRNL.SYS
	mcopy -i bin/OS.bin build/KRNLDR.SYS \:\:KRNLDR.SYS

run: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin

clean:
	rm -rf bin/* build/*

git:
	git add -A
	git commit -m "$m"
	git push
