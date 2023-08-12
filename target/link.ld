ENTRY(start)                /* the name of the entry label */
OUTPUT_ARCH(i386)

SECTIONS 
{
    . = 0x100000;

    .text ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.text*)             /* all text sections from all files */
    }

    .data ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.data*)             /* all data sections from all files */
    }

    .rodata ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.rodata*)             /* all read-only data sections from all files */
    }

    .bss ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.bss*)             /* all bss sections from all files */
    }
}
