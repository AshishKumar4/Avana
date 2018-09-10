#include "Hardware/vga/vga.h"
#include "Arch/x86.h"

extern uint32_t consolerow;
extern uint32_t consolecolumn;
extern uint32_t console_rows_skipped;
extern uint8_t console_color;
extern uint8_t default_console_color;
extern uint16_t *vgamem;
extern uint16_t *console_buffer;
extern uint16_t *console_dbuffer;  // Pointer to VGA Buffer current start offset

int vga_init()
{
    console_color=make_color(COLOR_WHITE,COLOR_BLACK);  // Generate Console Color Word
    default_console_color = console_color;
    for(int i=0;i<VGA_HEIGHT;i++)   // Basically, Clear Screen
    {
        for(int j=0;j<VGA_WIDTH;j++)    
        {
            console_buffer[i*VGA_WIDTH+j]=make_vgaentry(' ', console_color);        // Mark each cell with blank 'Spaces'
        }   
    }
    console_dbuffer = console_buffer;
    return 0;
}

void vga_setCursor(int row, int col)
{
   unsigned short position=(row*80) + col;

   // cursor LOW port to vga INDEX register
   outb(0x3D4, 0x0F);
   outb(0x3D5, ((unsigned char)(position&0xFF)));
   // cursor HIGH port to vga INDEX register
   outb(0x3D4, 0x0E);
   outb(0x3D5, ((unsigned char )((position>>8)&0xFF)));
}

void vga_updateCursor()
{
	vga_setCursor(consolerow, consolecolumn);
}

inline void vga_setcolor(uint8_t color)
{
	console_color = color;  // Sets the console color for future characters
}

void vga_putentryat(char c, uint8_t color, size_t x, size_t y)      // puts a character at a given x and y coordinate with given color
{
	const size_t index = y * VGA_WIDTH + x;
	console_dbuffer[index] = make_vgaentry(c, color);
}

int vga_putchar(char ch)   // Puts a character on the console next to the last character, filling the console linearly
{
	if(ch == '\n')   // If New line character
	{
		consolecolumn = 0;
		++consolerow;
		if(consolerow >= VGA_HEIGHT)
		{
			++console_rows_skipped;
            for(int i = 0; i < (VGA_WIDTH * (VGA_HEIGHT)); i++)     // Shift the complete console screen a bit up
            {
                console_dbuffer[i] = console_dbuffer[i + VGA_WIDTH];      
            }
			--consolerow;
		}
		vga_updateCursor();
		return (int)ch;  // Nothing needs to be printed
	}
	else if(ch == '\t')      // If Tab Character
	{
		consolecolumn += 5;
	}
	else    // Normal character
	{
		vga_putentryat(ch, console_color, consolecolumn, consolerow);
	}
	if(++consolecolumn >= VGA_WIDTH)    // Screen is full horizontally
	{
		consolecolumn = 0;
		++consolerow;
		if(consolerow >= VGA_HEIGHT)    // Complete screen is full
		{
            for(int i = 0; i < (VGA_WIDTH * (VGA_HEIGHT)); i++)     // Shift the complete console screen a bit up
            {
                console_dbuffer[i] = console_dbuffer[i + VGA_WIDTH];      
            }
			--consolerow;
		}
		vga_updateCursor();
		return (int)ch;
	}
	vga_updateCursor();
	return (int)ch;
}


void vga_putstring(char* str, int len)      // Print a string
{
    for(int i = 0; i < len; i++)
    {
        vga_putchar(str[i]);
    }
}

void vga_putstring_auto(char* str)      // Print a string without knowing its length
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        vga_putchar(str[i]);
    }
}

void backspace()
{
		--consolecolumn;
		if(!consolecolumn)
		{
			--consolerow;
			consolecolumn = VGA_WIDTH;
		}
		vga_putentryat(' ', console_color, consolecolumn, consolerow);
}