section .text

global jmp_usermode
extern userland

jmp_usermode:
    cli
    mov ax, 0x23 ; user mode data selector is 0x20 (GDT entry 3), with RPL 3 (ring 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23	; SS
    push esp	; ESP
    pushf	; EFLAGS

    pop eax
    or eax, 0x200   ; enable IF in EFLAGS
    push eax

    push 0x1B	    ; CS, user mode code selector is 0x18, with RPL 3 (ring 3)
    push userland   ; EIP
    iret
