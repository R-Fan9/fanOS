bits 32

org 0x10000	    ; kernel starts at 1 MB

start:
    jmp	    main

%include "stdio.inc"
%include "gtd.inc"

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
