ENTRY(start)                /* the name of the entry label */
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

SECTIONS 
{
    . = 0x100000;

    .text ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.text)             /* all text sections from all files */
    }
}
