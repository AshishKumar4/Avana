#pragma once 

#include "stdint.h"
#include "Arch/x86/custom_defs.h"

int init_cpu_x86();

uint32_t readl(uint32_t addr);
void writel(uint32_t addr, uint32_t val);
void delay(uint32_t delay);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void io_wait(void);
bool are_interrupts_enabled();
void lidt_n(void* base, uint16_t size);
void sysManager(unsigned int todo);
void stosb(void *addr, int data, int cnt);
void stosl(void *addr, int data, int cnt);
void ltr(uint16_t sel);
uint32_t readeflags(void);
void loadgs(uint16_t v);
uint32_t xchg(volatile uint32_t *addr, uint32_t newval);
uint32_t rcr2(void);
void lcr3(uint32_t val);

uint32_t StrToInt(char *str);