void kmain(void)
{
    char *vga = (char *)0xB8000;
    vga[0] = 'X';
    vga[1] = 0x28;
}
