;*******************************************************
;
;	Fat12.inc
;		FAT12 filesystem for 3-1/2 floppies
;
;*******************************************************

%ifndef __FAT12_INC_67343546FDCC56AAB872_INCLUDED__
%define __FAT12_INC_67343546FDCC56AAB872_INCLUDED__

bits 16

%include "floppy16.inc"

%define ROOT_OFFSET 0x2E00
%define FAT_SEG 0x2C0
%define ROOT_SEG 0x2E0

;*******************************************
; load_root ()
;	- Load Root Directory Table to 0x7E00
;*******************************************
load_root:
	pusha
	push		es

	; compute the size of root directory in sectors, and store it in CX

	xor		cx, cx
	xor		dx, dx
	mov		ax, 0x0020					; every entry is 32 bytes
	mul		WORD [bpbRootEntries]	    ; total size of root directory in bytes
	div		WORD [bpbBytesPerSector]    ; sectors in root directory
	xchg	ax, cx

	; compute the location of root directory, and store it in AX

	mov		al, BYTE [bpbNumberOfFATs]  ; number of FATs 
	mul		WORD [bpbSectorsPerFAT]	    ; sectors used by FATs
	add		ax, WORD [bpbReservedSectors]

	mov		WORD [datasector], ax		; base of root directory
	add		WORD [datasector], cx

	; load root directory into memory (0x7E00:0000)

	push	ROOT_SEG
	pop		es
	mov		bx, 0
	call	read_sectors
	pop		es
	popa
	ret

;*******************************************
; load_FAT ()
;	- Loads FAT table to 0x7C00
;
;	Parm/ ES:DI => Root Directory Table
;*******************************************
load_FAT:
	pusha
	push	es

	; compute the size of FAT in sectors, and store it in CX
	
	xor		ax, ax
	mov		al, BYTE [bpbNumberOfFATs]
	mul		WORD [bpbSectorsPerFAT]
	mov		cx, ax

	; compute the location of FAT, and store it in AX

	mov		ax, WORD [bpbReservedSectors]

	; read FAT into memory (0x7C00:0000)

	push	FAT_SEG
	pop		es
	xor		bx, bx
	call	read_sectors
	pop		es
	popa
	ret

;*******************************************
; find_image ()
;	- Search for filename in root table
;
; parm/ DS:SI => Image name
; ret/ AX => File index number in directory table. -1 if error
;*******************************************
find_image:
	push	cx
	push	dx
	push	bx
	
	mov		bx, si		; store the image name in BX

	; browse root directory for binary image

	mov		cx, WORD [bpbRootEntries]		; load loop counter
	mov		di, ROOT_OFFSET					; locate first root directort entrt
	cld		; clear direction flag

.loop:
	push	cx
	mov		cx, 0x000B		; 11 character name
	mov		si, bx			; image name is in BX
	push	di
	rep	cmpsb				; test if entry matches
	pop		di
	je		.found
	pop		cx
	add		di, 0x0020		; queue next directory entry
	loop	.loop

.not_found:
	pop		bx
	pop		dx
	pop		cx
	mov		ax, -1			; set error code
	ret

.found:
	pop		ax				; return value into AX contains entry of file
	pop		bx
	pop		dx
	pop		cx
	ret
		
;*******************************************
; load_image ()
;	- Load limage

; parm/ ES:SI => File to load
; parm/ EBX:BP => Buffer to load file to
; ret/ AX => -1 on error, 0 on success
; ret/ CX => number of sectors read
;*******************************************

load_image:
	xor		ecx, ecx
	push	ecx

.find_image:
	push	bx				; BX => BP points to the buffer to write to; store it for later
	push	bp

	call	find_image		; find image, ES:SI contains the image name	

	cmp		ax, -1
	jne		.load_image_pre
	pop		bp
	pop		bx
	pop		ecx
	mov		ax, -1
	ret

.load_image_pre:
	sub		edi, ROOT_OFFSET
	sub		eax, ROOT_OFFSET

	; get first cluster

	push	WORD ROOT_SET
	pop		es
	mov		dx, WORD [es:di + 0x001A]		; firster cluster of every entry is a byte 16
	mov		WORD [cluster], dx
	pop		bx		; get location to write to
	pop		es
	push	bx
	push	es
	call	load_FAT

.load_image:
	
	; load the cluster
	mov		ax, WORD [cluster]
	pop		es				; BX:BP = EX:BX
	pop		bx
	call	ClusterLBA
	xor		cx, cx			; setup the number of sectors to read
	mov		cl, bYTE [bpbSectorsPerCluster]
	call	read_sectors
	pop		ecx
	inc		ecx				; add one more sector to counter
	push	ecx
	push	bx
	push	es
	mov		ax, FAT_SET		; start reading from FAT
	mov		es, ax
	xor		bx, bx

	; get next cluster

	mov		ax, WORD [cluster]
	mov		cx, ax
	mov		dx, ax
	shr		dx, 0x0001		; divide by 2
	add		cx, dx			; sum for (3/2)

	mov		bx, 0			; location of FAT in memory (ES:BX)
	add		bx, cx
	mov		dx, WORD [es:bx]	; read 2 bytes from FAT
	test	ax, 0x0001
	jnz		.odd_cluster
	
.even_cluster:
	and		dx, 0000111111111111b		; take lower 12 bits
	jmp		.done

.odd_cluster:
	shr		dx, 0x0004					; take higher 12 bits

.done:
	mov		WORD [cluster], dx
	cmp		dx, 0x0FF0					; test for end of image
	jb		.load_image

.success:
	pop		es
	pop		bx
	pop		ecx
	xor		ax, ax
	ret

%endif	;__FAT12_INC_67343546FDCC56AAB872_INCLUDED__
