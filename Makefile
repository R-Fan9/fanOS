AS = nasm
AFLAGS = -f bin -i src/boot/include

QEMU = qemu-system-i386 
QFLAGS = -fda 

boot_bin_files = build/Boot.bin build/KRNLDR.SYS build/KRNL.SYS

build/KRNL.SYS: src/boot/stage3.asm
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o build/KRNL.SYS

build/KRNLDR.SYS: src/boot/stage2.asm
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o build/KRNLDR.SYS

build/Boot.bin: src/boot/boot.asm
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $< -o build/Boot.bin

bin/OS.bin: $(boot_bin_files)
	mkdir -p $(dir $@)
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
