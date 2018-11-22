#ifndef CONSOLE_HANDLE_H
#define CONSOLE_HANDLE_H

#include "stdlib.h"
#include "stdio.h"
#include "Processing/tasking.h"
#include "../../std_iohandling.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Memory/mem.h"
#include "../../Serial_Processing/serials.h"

volatile uint32_t console_end = 0, console_q_elements = 0;
volatile serials_o_struct_t* console_Start_q, *console_Current_q, *console_Last_q;

void console_io_init();
void _stdprint(const char* str, uint32_t length);

#endif
