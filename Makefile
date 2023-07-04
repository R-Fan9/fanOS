AS = nasm
AFLAGS = -f bin

QEMU = qemu-system-i386 
QFLAGS = -fda 

OBJECTS=Boot.bin KRNLDR.SYS KRNL.SYS

bin/OS.bin: $(OBJECTS)
	dd if=/dev/zero of=bin/OS.bin bs=512   count=2880           # floppy is 2880 sectors of 512 bytes
	dd if=Boot.bin of=bin/OS.bin seek=0 count=1 conv=notrunc   # Add boot BIN
	mcopy -i bin/OS.bin KRNL.SYS \:\:KRNL.SYS
	mcopy -i bin/OS.bin KRNLDR.SYS \:\:KRNLDR.SYS

KRNL.SYS: stage3.asm
	$(AS) $(AFLAGS) $< -o KRNL.SYS

KRNLDR.SYS: stage2.asm
	$(AS) $(AFLAGS) $< -o KRNLDR.SYS

Boot.bin: boot.asm
	$(AS) $(AFLAGS) $< -o Boot.bin

run: bin/OS.bin
	$(QEMU) $(QFLAGS) bin/OS.bin

clean:
	rm -rf bin/* *.bin *.SYS

git:
	git add -A
	git commit -m "$m"
	git push
