#pragma once

#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

void init_timer_RTC();
void init_timer(uint32_t frequency);
void delay1(uint32_t ms);