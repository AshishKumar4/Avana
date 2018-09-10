#include "Console/Console.h"
#include "Arch/x86.h"
#include "stdio.h"

/*
    At the system initialization, We shall start with a VGA based simple console screen.
*/

extern intfunc1_t putchar;

int init_console()
{
    consolerow = 0;
    consolecolumn = 0;
    console_rows_skipped = 0;
    console_color = 0;
    vgamem = (uint16_t *)0xC00B8000;//0xb8000;
    console_buffer = (uint16_t *)0xC00B8000;//0xb8000;
    vga_init();
    putchar = (intfunc1_t)&vga_putchar;
    printf("Console Initialized... Hello World");
    return 0;
}