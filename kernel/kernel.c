void kentr(void)
{
    char *vidptr = (char*)0xb8000;
    vidptr[0] = 'H';
    vidptr[2] = 'e';
    vidptr[4] = 'l';
    vidptr[6] = 'l';
    vidptr[8] = 'o';
    return;
}