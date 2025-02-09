int main()
{
    unsigned char *vga = (unsigned char*)0xb8000;
    vga[0] = '!';
}