AS = nasm
ASFLAGS = -f bin -i include/

boot_files := build/Boot.bin build/KRNLDR.SYS

all: $(boot_files)

build/KRNLDR.SYS: stage2.asm
	$(AS) $(ASFLAGS) $< -o $@

build/Boot.bin: boot.asm
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: all
