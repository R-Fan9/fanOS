AS = nasm
ASFLAGS = -f elf32

CC = gcc
CINCLUDE = -I src/include
CFLAGS = $(CINCLUDE) -std=c17 -m32 -march=i386 -mgeneral-regs-only -ffreestanding -fno-stack-protector -fno-builtin -nostdinc -nostartfiles -nodefaultlibs -fno-PIC -fno-pie -Wall -Wextra -Wno-pointer-sign -Wno-interrupt-service-routine -g -c

LD = ld
LFLAGS = -m elf_i386 --oformat binary

QEMU = qemu-system-i386 
QFLAGS = -monitor stdio -fda

BOCHS = bochs
BFLAGS = -qf

bootloader := boot/build/Boot.bin boot/build/KRNLDR.SYS

include_source_files := $(shell find src/include/ -name *.c)
include_object_files := $(patsubst src/include/%.c, build/include/%.o, $(include_source_files))

$(include_object_files): build/include/%.o : src/include/%.c
	mkdir -p $(dir $@) && \
	$(CC) $(CFLAGS) $(patsubst build/include/%.o, src/include/%.c, $@) -o $@

build/stage3.o: src/stage3.c
	mkdir -p $(dir $@) && \
	$(CC) $(CFLAGS) $^ -o $@

build/kernel.o: src/kernel.c
	mkdir -p $(dir $@) && \
	$(CC) $(CFLAGS) $^ -o $@

build/PRKRNL.SYS: build/stage3.o $(include_object_files)
	mkdir -p $(dir $@) && \
	$(LD) $(LFLAGS) -T target/stage3.ld -o $@ $^

build/KRNL.SYS: build/kernel.o $(include_object_files)
	mkdir -p $(dir $@) && \
	$(LD) $(LFLAGS) -T target/kernel.ld -o $@ $^

$(bootloader):
	$(MAKE) -C boot all

bin/OS.bin: $(bootloader) build/PRKRNL.SYS 
	mkdir -p $(dir $@) && \
	dd if=/dev/zero of=bin/OS.bin bs=512   count=2880           # floppy is 2880 sectors of 512 bytes
	dd if=boot/build/Boot.bin of=bin/OS.bin seek=0 count=1 conv=notrunc
	mcopy -i bin/OS.bin boot/build/KRNLDR.SYS \:\:KRNLDR.SYS
	mcopy -i bin/OS.bin build/PRKRNL.SYS \:\:PRKRNL.SYS

run: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin

debug: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin -S -s

bochs: bin/OS.bin
	$(BOCHS) $(BFLAGS) .bochsrc

clean:
	rm -rf bin/* build/* && \
	$(MAKE) -C boot clean

git:
	git add -A
	git commit -m "$m"
	git push
