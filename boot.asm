use16	
;; 16 bit real mode
	org	0x7C00		; Boot code is loaded by BIOS at 0x7C00

	start:
		jmp		loader
	
	loader:
		xor		ax, ax		; Ensure data & extra segments are 0 to start
		mov		ds, ax
		mov		es, ax

		cli		; Clear all interrupts
		hlt		; Halt the system
		

	;; Boot sector magic
	times 510-($-$$) db 0	; Pads out 0s until the 510th byte is reached
	dw 0xAA55		; BIOS magic number; Boot signiture
