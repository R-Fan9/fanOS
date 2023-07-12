section .text

bits 32

global start
extern main

start:
    jmp	    enter_kernel

;*******************************************************
;	KERNEL ENTRY POINT
;*******************************************************

enter_kernel:
    
    ; set up segment registers
    mov	    ax, 0x10
    mov	    ds, ax
    mov	    es, ax

    ; set up stack
    mov	    ss, ax
    mov	    esp, 0x90000	; stack pointer begins at 0x90000
    
    call    main

    cli
    hlt
