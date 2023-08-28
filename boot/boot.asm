bits 16	    ; 16 bits real mode

org 0x0

start:
    jmp     main

;*********************************************
;	BIOS Parameter Block
;*********************************************
bpbOEM			db "X86 OS  "			; OEM identifier (Cannot exceed 8 bytes!)
bpbBytesPerSector:  	DW 512
bpbSectorsPerCluster: 	DB 1
bpbReservedSectors: 	DW 1
bpbNumberOfFATs: 	DB 2
bpbRootEntries: 	DW 224
bpbTotalSectors: 	DW 2880
bpbMedia: 		DB 0xf8  ;; 0xF1
bpbSectorsPerFAT: 	DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 	DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 		DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "MOS FLOPPY "
bsFileSystem: 	        DB "FAT12   "

;************************************************;
; Convert CHS to LBA
; LBA = (cluster - 2) * sectors per cluster
;************************************************;
ClusterLBA:
     sub     ax, 0x0002				; zero base cluster number
     xor     cx, cx
     mov     cl, BYTE [bpbSectorsPerCluster]	; convert byte to word
     mul     cx
     add     ax, WORD [datasector]		; base data sector
     ret
     
;************************************************;
; Convert LBA to CHS
; AX=>LBA Address to convert
;
; absolute sector = (logical sector / sectors per track) + 1
; absolute head   = (logical sector / sectors per track) MOD number of heads
; absolute track  = logical sector / (sectors per track * number of heads)
;
;************************************************;
LBACHS:
     xor     dx, dx			    ; prepare dx:ax for operation
     div     WORD [bpbSectorsPerTrack]	    ; calculate
     inc     dl				    ; adjust for sector 0
     mov     BYTE [absoluteSector], dl
     xor     dx, dx			    ; prepare dx:ax for operation
     div     WORD [bpbHeadsPerCylinder]	    ; calculate
     mov     BYTE [absoluteHead], dl
     mov     BYTE [absoluteTrack], al
     ret

;************************************************;
; Reads a series of sectors
; CX=>Number of sectors to read
; AX=>Starting sector
; ES:BX=>Buffer to read to
;************************************************;
read_sectors:
     .main:
          mov     di, 0x0005	    ; five retries for error

     .sector_loop:
          push    ax
          push    bx
          push    cx
          call    LBACHS            ; convert starting sector to CHS

          mov     ah, 0x02	    ; BIOS read sector
          mov     al, 0x01          ; read one sector
          mov     ch, BYTE [absoluteTrack]            ; track
          mov     cl, BYTE [absoluteSector]           ; sector
          mov     dh, BYTE [absoluteHead]             ; head
          mov     dl, BYTE [bsDriveNumber]            ; drive
          int     0x13		    ; invoke BIOS
          jnc     .success          ; test for read error

          xor     ax, ax	    ; BIOS reset disk
          int     0x13              ; invoke BIOS
          dec     di                ; decrement error counter
          pop     cx
          pop     bx
          pop     ax
          jnz     .sector_loop	    ; attempt to read again
          int     0x18

     .success:
          pop     cx
          pop     bx
          pop     ax
          add     bx, WORD [bpbBytesPerSector]        ; queue next buffer
          inc     ax                                  ; queue next sector
          loop    .main                               ; read next sector
          ret


;*********************************************
;	BOOTLOADER ENTRY POINT
;
;	    -Load root directory
;	    -Find Stage 2 image
;	    -Load FAT
;	    -Jump to Stage 2
;*********************************************
main:

    ;----------------------------------------------------
    ; code located at 0000:7C00, adjust segment registers
    ;----------------------------------------------------
    cli			    ; clear interrupts
    mov	    ax, 0x07C0	    ; set up registers to point to our segment
    mov	    ds, ax
    mov	    es, ax
    mov	    fs, ax
    mov	    gs, ax

    ;----------------------------------------------------
    ; Create stack
    ;----------------------------------------------------
    mov	    ax, 0x0000
    mov	    ss, ax
    mov	    sp, 0xFFFF
    sti			    ; restore interupts

    ;----------------------------------------------------
    ; Load root directory
    ;----------------------------------------------------
    load_root:
	
	; compute the size of root directory in sectors, and store it in CX
	xor	cx, cx
	xor	dx, dx
	mov	ax, 0x0020		    ; every entry is 32 bytes
	mul	WORD [bpbRootEntries]	    ; total size of root directory in bytes
	div	WORD [bpbBytesPerSector]    ; sectors in root directory
	xchg	ax, cx

	; compute the location of root directory, and store it in AX
	mov	al, BYTE [bpbNumberOfFATs]  ; number of FATs 
	mul	WORD [bpbSectorsPerFAT]	    ; sectors used by FATs
	add	ax, WORD [bpbReservedSectors]
	
	mov	WORD [datasector], ax
	add	WORD [datasector], cx

	; load root directory into memory (7C00:0200)
	mov	bx, 0x0200
	call	read_sectors

    ;----------------------------------------------------
    ; Find stage 2
    ;----------------------------------------------------
	mov	cx, WORD [bpbRootEntries]	; load loop counter
	mov	di, 0x0200			; locate first root directory
    .loop:
	push	cx
	mov	cx, 0x000B	; 11 characters file name
	mov	si, imageName
	push	di
	rep cmpsb			; test if entry matches
	pop	di
	je	load_FAT
	    pop	cx
	add	di, 0x0020	; move to next entry by incrementing 32 bytes
	loop	.loop
	jmp	failure

    ;----------------------------------------------------
    ; Load FAT
    ;----------------------------------------------------
    load_FAT:
	
	; save first cluster of boot image
	mov	dx, WORD [di+0x001A]	    ; first cluster of every entry is at byte 26
	mov	WORD [cluster], dx

	; compute the size of FAT in sectors, and store it in CX
	xor	ax, ax
	mov	al, BYTE [bpbNumberOfFATs]
	mul	WORD [bpbSectorsPerFAT]
	mov	cx, ax

	; compute the location of FAT, and store it in AX
	mov	ax, WORD [bpbReservedSectors]

	; read FAT into memory (7C00:0200)
	mov	bx, 0x0200
	call	read_sectors

	; read image file into memory (0050:0000)
	mov	ax, 0x0050
	mov	es, ax
	xor	bx, bx
	push	bx
	
    ;----------------------------------------------------
    ; Load stage 2
    ;----------------------------------------------------
    load_image:
	mov	ax, WORD [cluster]	; cluster to read
	pop	bx
	call	ClusterLBA		; convert cluster to LBA
	xor	cx, cx
	mov	cl, BYTE [bpbSectorsPerCluster]
	call	read_sectors
	push	bx

	; compute next cluster
	mov	ax, WORD[cluster]
	mov	cx, ax
	mov	dx, ax	
	shr	dx, 0x0001	; divide by 2
	add	cx, dx		; sum for (3/2)
	mov	bx, 0x0200	; location of FAT in memory
	add	bx, cx		; index into FAT
	mov	dx, WORD [bx]	; read 2 bytes from FAT
	test	ax, 0x0001
	jnz	.odd_cluster

	.even_cluster:
	    and	    dx, 0000111111111111b   ; takes the lower 12 bits
	    jmp	    .done

	.odd_cluster:
	    shr	    dx, 0x00004		    ; takes the higher 12 bits

	.done:
	    mov	    WORD [cluster], dx	    ; stores new cluster
	    cmp	    dx, 0x0FF0		    ; test end of the file	
	    jb	    load_image
	
    done:
	push	WORD 0x0050
	push	WORD 0x0000
	retf

    failure:
	mov	ah, 0x00
	int	0x16	    ; waits for keypress
	int	0x19	    ; warm boot the computer

;*************************************************;
;   Data section
;*************************************************;
absoluteSector db 0x00
absoluteHead   db 0x00
absoluteTrack  db 0x00

datasector  dw 0x0000 
cluster     dw 0x0000 
imageName   db "KRNLDR  SYS"

; Boot Sector magic
times 510-($-$$) db 0
dw 0xAA55
