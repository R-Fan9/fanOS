;*************************************************
;	stdio.inc
;		-Input/Output routines
;
;*************************************************
 
%ifndef __STDIO_INC_67343546FDCC56AAB872_INCLUDED_ 
%define __STDIO_INC_67343546FDCC56AAB872_INCLUDED_ 

;************************************************;
;	print_str ()
;		-Prints a null terminated string
;	DS=>SI: 0 terminated string
;************************************************;

bits 16
print_str:

    pusha                   ; save registers

    .loop:
        lodsb                   ; load the next the character
        or      al, al          ; test if AL == 0?
        jz      .print_done     ; yes - end of the strin
        mov     ah, 0xE         ; no - print the character
        int     0x10
        jmp     .loop

    .print_done
        popa                    ; restore registers
        ret
        


%endif ;__STDIO_INC_67343546FDCC56AAB872_INCLUDED__
