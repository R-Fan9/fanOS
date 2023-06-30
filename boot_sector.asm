use16	
;; 16 bit real mode
	org	0x7C00		; Boot code is loaded by BIOS at 0x7C00

	start:
		jmp		loader
	
	loader:
		xor		ax, ax		; Ensure data & extra segments are 0 to start
		mov		ds, ax
		mov		es, ax

	.reset:
		mov		ah, 0x0		; Reset floppy disk function
		mov		dl, 0		; floppy disk is drive 0
		int		0x13		; call BIOS interrupt 0x13
		jc		.reset		; if carry bit is 1, there is error. Try again
	
	.read:
		mov		ah, 0x2		; Read sector function
		mov		al, 1		; Read 1 sector
		mov		ch, 1		; Read from track 1
		mov		cl, 2		; Read from sector 2
		mov		dh, 0		; Head number 0
		mov		dl, 0		; floppy disk is drive 0

		int		0x13		; call BIOS interrupt 0x13
		jc		.read		; if carry bit is 1, there is error. Try again

		jmp		0x1000:0x0
		

	;; Boot sector magic
	times 510-($-$$) db 0	; Pads out 0s until the 510th byte is reached
	dw 0xAA55		; BIOS magic number; Boot signiture
	
	; End of sector 1, beginning of sector 2 -----------------------------

	org 0x1000

	cli		; Clear all interrupts
	hlt		; Halt the system
