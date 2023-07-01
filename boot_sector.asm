bits 16                 ; 16 bits real mode

org 0x7C00

start:
    jmp     loader

print_str:
    lodsb               ; move next byte from string from SI to AL
    or      al, al      ; check if AL == 0
    jz      print_done  ; yes - reached the end of the string, done printing
    mov     ah, 0xE     ; no - print the character 
    int     0x10
    jmp     print_str
    
print_done:
    ret

loader:
    xor     ax, ax
    mov     ds, ax
    mov     es, ax

    mov     si, msg
    call    print_str

    xor     ax, ax

    cli
    hlt

.reset:
    mov     ah, 0x0     ; read floppy disk function
    mov     dl, 0x0     ; drive 0 is floppy disk
    int     0x13
    jc      .reset      ; carry flag is set - error try again

    mov     ax, 0x1000  ; read sector into address 0x1000:0
    mov     es, ax
    xor     bx, bx

.read:
    mov     ah, 0x02    ; read sector function
    mov     al, 1       ; read 1 sector
    mov     ch, 1       ; track 1 
    mov     cl, 2       ; sector 2
    mov     dh, 0       ; head 0
    mov     dl, 0       ; drive 0

    int     0x13
    jc      .read       ; carry flag is set - error try again

    jmp     0x1000:0x0

msg	db	"Welcome to My Operating System!", 0

times 510-($-$$) db 0
dw 0xAA55
