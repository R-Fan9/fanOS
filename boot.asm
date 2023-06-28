org	0x7C00		; Boot code is loaded by BIOS at 0x7C00

bits	16		; Currently in 16 bit real mode

start:
	cli		; Clear all interrupts
	hlt		; halt the system

times 510-($-$$) db 0	; Pads out 0s until the 510th byte is reached
dw 0xAA55		; BIOS magic number; Boot signiture
