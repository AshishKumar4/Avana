#pragma once 

#include "Arch/x86.h"

uint32_t consolerow, consolecolumn, console_rows_skipped;
uint8_t console_color;
uint8_t default_console_color;
uint16_t *vgamem;
uint16_t *console_buffer;
uint16_t *console_dbuffer;  // Pointer to VGA Buffer current start offset

int init_console();
