bits 16

org 0x500

start:
    jmp	    main				


;*******************************************************
;	Preprocessor directives
;*******************************************************

%include "stdio.inc"	    ; basic i/o routines
%include "gdt.inc"	    ; GDT routines
%include "a20.inc"	    ; Gate A20 routines


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
    mov	    ax, 0x9000
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
    call enable_A20_kbrd_out

    mov	    si, msg
    call    print_str

    ;----------------------------------------------------
    ; Enable pmode
    ;----------------------------------------------------
    cli
    mov	    eax, cr0
    or	    eax, 1
    mov	    cr0, eax

    jmp CODE_DESC:stage_3	; code descriptor is 0x8


;*************************************************;
;   STAGE 3 ENTRY POINT
;*************************************************;

bits 32

stage_3:

    ; set data segments
    mov	    ax, DATA_DESC	; data descriptor ix 0x10
    mov	    ds, ax
    mov	    ss, ax
    mov	    es, ax
    mov	    esp, 0x90000	; stack begins from 0x90000 

stop:
    cli
    hlt

;*************************************************;
;   Data section
;*************************************************;
msg	db	"Welcome to 32 bits kernel!", 0
