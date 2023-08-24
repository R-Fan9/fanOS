bits 16

org 0x500

%define IMAGE_PMODE_BASE 0x50000   ; where the kernel is to be loaded to in protected mode
%define IMAGE_RMODE_BASE 0x3000    ; where the kernel is to be loaded to in real mode

start:
    jmp	    main				


;*******************************************************
;	preprocessor directives
;*******************************************************
%include "stdio.inc"	    ; basic i/o routines
%include "gdt.inc"	    ; GDT routines
%include "a20.inc"	    ; Gate A20 routines
%include "fat12.inc"	    ; FAT12 routines
%include "memory.inc"	    ; memory routines


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
    ; Get memory map 
    ;----------------------------------------------------
    memmap_entry_count equ 0x1000       ; store # of memory map entries here
    mov     di, 0x1004			; store memory map entries here
    call    get_memory_map
    mov     [memmap_entry_count], bp    ; store # of memory map entries when done

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
    mov	    si, loading_msg
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
    mov	    si, image_name
    call    load_image
    mov	    DWORD [image_size], ecx
    cmp	    ax, 0
    je	    enter_stage3
    mov	    si, failure_msg
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
    mov	    si, success_msg
    call    print_str

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
    mov	    es, ax
    mov	    fs, ax
    mov	    gs, ax
    mov	    ss, ax
    mov	    esp, 0x90000	; stack begins from 0x90000 

;----------------------------------------------------
; Copy kernel to 1MB
;----------------------------------------------------
copy_image:
    mov	    eax, DWORD [image_size]
    mov	    [0x8000], eax	; move the value of kernel image size to address 0x8000
    movzx   ebx, WORD [bpbBytesPerSector]
    mul	    ebx
    mov	    ebx, 4
    div	    ebx
    cld
    mov	    esi, IMAGE_RMODE_BASE
    mov	    edi, IMAGE_PMODE_BASE
    mov	    ecx, eax
    rep	    movsd		; copy image to its protected mode address
    
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
image_name   db "PRKRNL  SYS"
image_size   db 0
loading_msg	db	"Searching for Operating System...", 0x0A, 0x00
success_msg	db	"Found Operating System!!!", 0x0A, 0x00
failure_msg db 0x0D, 0x0A, "*** FATAL: Missing or corrupt KRNL.SYS. Press Any Key to Reboot.", 0x0D, 0x0A, 0x0A, 0x00

