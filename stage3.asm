bits 32

org 0x100000	    ; kernel starts at 1 MB

start:
    jmp	    main


;*******************************************************
;	preprocessor directives
;*******************************************************
%include "stdio.inc"
%include "gdt.inc"


;*******************************************************
;	STAGE 3 ENTRY POINT
;*******************************************************

main:
    
    ; set up segment registers
    mov	    ax, DATA_DESC
    mov	    ds, ax
    mov	    es, ax

    ; set up stack
    mov	    ss, ax
    mov	    esp, 0x90000	; stack pointer begins at 0x90000

    mov	    si, msg
    call    print_str

    cli
    hlt

;*************************************************;
;   Data section
;*************************************************;
msg	db	"Welcome to 32 bits kernel!", 0
