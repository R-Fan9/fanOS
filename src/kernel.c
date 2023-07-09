void main(void)
{
    char *vga = (char *)0xB8000;
    vga[0] = 'X';
}
