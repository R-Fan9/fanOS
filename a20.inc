;********************************************
;	Enable A20 address line
;********************************************

%ifndef __A20_INC_67343546FDCC56AAB872_INCLUDED__
%define __A20_INC_67343546FDCC56AAB872_INCLUDED_

bits 16

;----------------------------------------------
; Enables a20 line through keyboard controller
;----------------------------------------------

enable_A20_kbrd:
	cli
	push	ax
	mov		al, 0xDD		; send enable A20 address line command to controller
	out		0x64, al
	pop		ax
	ret

;--------------------------------------------
; Enables a20 line through output port
;--------------------------------------------

enable_A20_kbrd_out:
	cli
	pusha
	
	; disable keyboard
	call	wait_input
	mov		al, 0xAD
	out		0x64, al

	; enable read output port command
	call	wait_input
	mov		al, 0xD0
	out		0x64, al

	; read data from output port
	call	wait_output
	in		al, 0x60
	push	eax

	; enable write output port command
	call	wait_input
	mov		al, 0xD1
	out		0x64, al

	; write data to output port
	call	wait_input
	pop		eax
	or		al, 2
	out		0x60, al

	; enable keyboard
	call	wait_input
	mov		al, 0xAE
	out		0x64, al

	call	wait_input
	popa
	sti
	ret

wait_input:
	in		al, 0x64
	test	al, 2h
	jnz		wait_input		; input buffer is full, dont write
	ret

wait_output:
	in		al, 0x64
	test	al, 1h
	jz		wait_output		; output buffer is empty, dont read
	ret

;--------------------------------------
; Enables A20 line through bios
;-------------------------------------

enable_A20_BIOS:
	pusha
	mov		ax, 0x2401		; int 0x15 function 2401 - enable A20
	int		0x15
	popa
	ret

%endif ;__A20_INC_67343546FDCC56AAB872_INCLUDED__
