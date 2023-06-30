git:
	git add -A
	git commit -m "$m"
	git push

build:
	nasm -f bin boot_sector.asm -o boot_sector.bin

run: boot_sector.bin
	qemu-system-i386 -fda boot_sector.bin
