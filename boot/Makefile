AS = nasm
ASFLAGS = -f bin -i include/

boot_files := build/Boot.bin build/KRNLDR.SYS

all: $(boot_files)

build/KRNLDR.SYS: stage2.asm
	mkdir -p $(dir $@) && \
	$(AS) $(ASFLAGS) $< -o $@

build/Boot.bin: boot.asm
	mkdir -p $(dir $@) && \
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf build/*

.PHONY: all
