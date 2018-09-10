#pragma once

#include "stdint.h"
#include "stddef.h"

uintptr_t std_in;

int printf(const char* restrict format, ...);
int printf(const char* restrict format, ...);
uint8_t default_console_color;
void itoa(unsigned i,char* buf, unsigned base);
