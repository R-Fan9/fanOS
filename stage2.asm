bits 16

org 0x0

start:
    jmp	    main				

print_str:
    lodsb                   ; move next byte from string from SI to AL
    or      al, al          ; check if AL == 0
    jz      .print_done     ; yes - reached the end of the string, done printing
    mov     ah, 0xE         ; no - print the character 
    int     0x10
    jmp     print_str
    
    .print_done:
        ret

;*******************************************************
;	STAGE 2 ENTRY POINT
;
;		-Store BIOS information
;		-Load Kernel
;		-Install GDT; go into protected mode (pmode)
;		-Jump to Stage 3
;*******************************************************

main:

    ; setup segments and stack
    cli				; clear interrupts
    mov	    ax, 0x0050		; null segments
    mov	    ds, ax
    mov	    es, ax

    mov	    si, msg
    call    print_str

    sti				; enable interrupts
    hlt

;*************************************************;
;   Data section
;*************************************************;
msg	db	"Welcome to My Operating System!", 0
