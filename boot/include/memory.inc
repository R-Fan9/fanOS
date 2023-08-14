;*************************************************
;	Memory.inc
;		-Basic memory routines
;
;*************************************************

%ifndef __MEMORY_INC_67343546FDCC56AAB872_INCLUDED__
%define __MEMORY_INC_67343546FDCC56AAB872_INCLUDED__

bits 16

;---------------------------------------------
;	Get memory map from bios
;	/in es:di->destination buffer for entries
;	/ret bp=entry count
;---------------------------------------------

get_memory_map
	xor		ebx, ebx
	xor		bp, bp					; number of entries stored here
	mov		edx, 'PAMS'				; 'SMAP'
	mov		eax, 0xE820
	mov		[es:di + 20], DWORD 1	; force a valie ACPI 3.x entry
	mov		ecx, 24					; memory map entry struct is 24 bytes
	int		0x15					; get first entry
	jc		.error
	cmp		eax, 'PAMS'				; bios returns SMAP in eax
	jne		.error
	test	ebx, ebx				; if ebx=0 then list is one entry long; bail out
	je		.error
	jmp		.start

.next_entry:
	mov		edx, 'PAMS'			; some bios's trash this register
	mov		ecx, 24				; entry is 24 bytes
	mov		eax, 0xE820
	int		0x15				; get next entry

.start:
	jcxz	.skip_entry         ; memory map entry is 0 bytes in length, skip

.notext:
	mov		ecx, [ES:DI + 8]    ; low 32 bits of length
	test		ecx, ecx
	jne		short .good_entry
	mov		ecx, [ES:DI + 12]	; or with high 32 bits of length, or will also set the ZF
	jecxz		.skip_entry         ; Length of memory region returned = 0, skip

.good_entry:
	inc		bp					; increment entry count
	add		di, 24				; point di to next entry in buffer

.skip_entry:
	cmp		ebx, 0					; if ebx return is 0, list is done
	jne		.next_entry				; get next entry
	jmp		.done

.error:
	stc
	ret

.done:
	clc
	ret

%endif
