AS = nasm
ASFLAGS = -f elf32

CC = gcc
CINCLUDE = -I src/include
CFLAGS = $(CINCLUDE) -std=c17 -m32 -march=i386 -mgeneral-regs-only -ffreestanding -fno-stack-protector -fno-builtin -nostdinc -nostartfiles -nodefaultlibs -fno-PIC -fno-pie -Wall -Wextra -Wno-pointer-sign -Wno-interrupt-service-routine -g -c

LD = ld
LFLAGS = -m elf_i386

QEMU = qemu-system-i386 
QFLAGS = -monitor stdio -fda 

bootloader := boot/build/Boot.bin boot/build/KRNLDR.SYS

asm_source_files := $(shell find src/ -name *.asm)
asm_object_files := $(patsubst src/%.asm, build/%.o, $(asm_source_files))

c_source_files := $(shell find src/ -name *.c)
c_object_files := $(patsubst src/%.c, build/%.o, $(c_source_files))

$(asm_object_files): build/%.o : src/%.asm
	mkdir -p $(dir $@) && \
	$(AS) $(ASFLAGS) $(patsubst build/%.o, src/%.asm, $@) -o $@

$(c_object_files): build/%.o : src/%.c
	mkdir -p $(dir $@) && \
	$(CC) $(CFLAGS) $(patsubst build/%.o, src/%.c, $@) -o $@

build/KRNL.SYS: $(asm_object_files) $(c_object_files)
	mkdir -p $(dir $@) && \
	$(LD) $(LFLAGS) -o $@ -T target/link.ld $^

$(bootloader):
	$(MAKE) -C boot all

bin/OS.bin: $(bootloader) build/KRNL.SYS 
	mkdir -p $(dir $@) && \
	dd if=/dev/zero of=bin/OS.bin bs=512   count=2880           # floppy is 2880 sectors of 512 bytes
	dd if=boot/build/Boot.bin of=bin/OS.bin seek=0 count=1 conv=notrunc
	mcopy -i bin/OS.bin boot/build/KRNLDR.SYS \:\:KRNLDR.SYS
	mcopy -i bin/OS.bin build/KRNL.SYS \:\:KRNL.SYS

run: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin

debug: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin -S -s

clean:
	rm -rf boot/build/* bin/* build/*

git:
	git add -A
	git commit -m "$m"
	git push
