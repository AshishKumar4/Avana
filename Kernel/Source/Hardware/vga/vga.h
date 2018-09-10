#pragma once 

#include "Arch/x86.h"

#define VGA_HEIGHT      25
#define VGA_WIDTH       80

enum vga_color
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

static inline uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

int vga_init();
inline void vga_setcolor(uint8_t color);	// Sets the console color for future characters
void vga_putentryat(char c, uint8_t color, size_t x, size_t y);      // puts a character at a given x and y coordinate with given color
int vga_putchar(char ch);   // Puts a character on the console next to the last character, filling the console linearly
void vga_putstring(char* str, int len);      // Print a string
void vga_putstring_auto(char* str);      // Print a string without knowing its length;
void backspace();