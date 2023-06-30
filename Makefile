AS = nasm
ASFLAGS = -f bin

QEMU = qemu-system-i386 
QFLAGS = -fda 

build:
	$(AS) $(ASFLAGS) boot_sector.asm -o boot_sector.bin

run: boot_sector.bin
	$(QEMU) $(QFLAGS) boot_sector.bin

git:
	git add -A
	git commit -m "$m"
	git push
