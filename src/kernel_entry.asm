section .text

bits 32

global start
global timer_irq0_handler
global keyboard_irq1_handler
extern main
extern timer_handler
extern keyboard_handler

start:
    jmp	    kernel

;*******************************************************
;	KERNEL ENTRY POINT
;*******************************************************

kernel:
    
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

timer_irq0_handler:
    add	    esp, 12
    pusha
    call    timer_handler
    popa
    iret

keyboard_irq1_handler:
    add	    esp, 12
    pusha
    cli
    call    keyboard_handler
    sti
    popa
    iret
