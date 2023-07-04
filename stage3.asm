bits 32

org 0x100000	    ; kernel starts at 1 MB

start:
    jmp	    main


;*******************************************************
;	preprocessor directives
;*******************************************************
%include "stdio.inc"


;*******************************************************
;	STAGE 3 ENTRY POINT
;*******************************************************

main:
    
    ; set up segment registers
    mov	    ax, 0x10
    mov	    ds, ax
    mov	    es, ax

    ; set up stack
    mov	    ss, ax
    mov	    esp, 0x90000	; stack pointer begins at 0x90000


    ;---------------------------------------;
    ;   Clear screen and print success
    ;---------------------------------------;
    call    ClrScr32
    mov	    ebx, msg
    call    Puts32

    cli
    hlt

;*************************************************;
;   Data section
;*************************************************;
msg db  0x0A, 0x0A, "                       - OS Development Series -"
    db  0x0A, 0x0A, "                     MOS 32 Bit Kernel Executing", 0x0A, 0
