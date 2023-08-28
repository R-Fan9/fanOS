SECTIONS
{
    .text 0x100000 :
    {
        *(kernel_main);
        *(.text*);
    }
    .data :
    {
        *(.data*);
    }
    .rodata :
    {
        *(.rodata*);
    }
    .bss :
    {
        *(.bss*);
    }
}
