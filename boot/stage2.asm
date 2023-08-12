bits 16

org 0x500

start:
    jmp	    main				


;*******************************************************
;	preprocessor directives
;*******************************************************
%include "common.inc"	    ; common routines
%include "stdio.inc"	    ; basic i/o routines
%include "gdt.inc"	    ; GDT routines
%include "a20.inc"	    ; Gate A20 routines
%include "fat12.inc"	    ; FAT12 routines


;*******************************************************
;	STAGE 2 ENTRY POINT
;
;	    -Store BIOS information
;	    -Install GDT; go into protected mode (pmode)
;	    -Jump to Stage 3 (kernel)
;*******************************************************

main:

    ; setup segments 
    cli				; clear interrupts
    xor	    ax, ax
    mov	    ds, ax
    mov	    es, ax

    ; setup stack
    mov	    ax, 0x0
    mov	    ss, ax
    mov	    sp, 0xFFFF
    sti				; enable interrupts

    ;----------------------------------------------------
    ; Install GDT 
    ;----------------------------------------------------
    call    install_GDT

    ;----------------------------------------------------
    ; Enable A20 
    ;----------------------------------------------------
    call    enable_A20_kbrd

    ;----------------------------------------------------
    ; Print loading message 
    ;----------------------------------------------------
    mov	    si, loadingMsg
    call    print_str

    ;----------------------------------------------------
    ; Load root directory
    ;----------------------------------------------------
    call    load_root

    ;----------------------------------------------------
    ; Load kernel
    ;----------------------------------------------------
    mov	    ebx, 0	; BX:BP points to buffer to load to
    mov	    bp, IMAGE_RMODE_BASE
    mov	    si, imageName
    call    load_image
    mov	    DWORD [imageSize], ecx
    cmp	    ax, 0
    je	    enter_stage3
    mov	    si, msgFailure
    call    print_str
    mov	    ah, 0x0
    int	    0x16	; wait for keypress
    int	    0x19	; warim boot compyter
    cli
    hlt

;----------------------------------------------------
; Enable pmode
;----------------------------------------------------
enter_stage3:

    cli
    mov	    eax, cr0
    or	    eax, 1
    mov	    cr0, eax

    jmp CODE_DESC:stage3	; code descriptor is 0x8


;*************************************************;
;   STAGE 3 ENTRY POINT
;*************************************************;

bits 32

stage3:

    ; setup registers
    mov	    ax, DATA_DESC	; data descriptor ix 0x10
    mov	    ds, ax

    ; set up stack
    mov	    ss, ax
    mov	    es, ax
    mov	    esp, 0x90000	; stack begins from 0x90000 

;----------------------------------------------------
; Copy kernel to 1MB
;----------------------------------------------------
copy_image:
    mov	    eax, DWORD [imageSize]
    movzx   ebx, WORD [bpbBytesPerSector]
    mul	    ebx
    mov	    ebx, 4
    div	    ebx
    cld
    mov	    esi, IMAGE_RMODE_BASE
    mov	    edi, IMAGE_PMODE_BASE
    mov	    ecx, eax
    rep	    movsd	; copy image to its protected mode address
    
    ;----------------------------------------------------
    ; Execute kernel
    ;----------------------------------------------------
    jmp    CODE_DESC:IMAGE_PMODE_BASE

stop:
    cli
    hlt

;*************************************************;
;   Data section
;*************************************************;
imageName   db "KRNL    SYS"
imageSize   db 0

loadingMsg	db	"Searching for Operating System...", 0x0A, 0x00
msgFailure db 0x0D, 0x0A, "*** FATAL: Missing or corrupt KRNL32.EXE. Press Any Key to Reboot.", 0x0D, 0x0A, 0x0A, 0x00

