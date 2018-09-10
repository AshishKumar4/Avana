#pragma once

#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

//Defines
#define KERNEL_MODE 0b00
#define USER_MODE 0b11


typedef volatile struct __tss_struct
{
    unsigned short   link;
    unsigned short   link_h;

    unsigned long   esp0;
    unsigned short   ss0;
    unsigned short   ss0_h;

    unsigned long   esp1;
    unsigned short   ss1;
    unsigned short   ss1_h;

    unsigned long   esp2;
    unsigned short   ss2;
    unsigned short   ss2_h;

    unsigned long   cr3;
    unsigned long   eip;
    unsigned long   eflags;

    unsigned long   eax;
    unsigned long   ecx;
    unsigned long   edx;
    unsigned long    ebx;

    unsigned long   esp;
    unsigned long   ebp;

    unsigned long   esi;
    unsigned long   edi;

    unsigned short   es;
    unsigned short   es_h;

    unsigned short   cs;
    unsigned short   cs_h;

    unsigned short   ss;
    unsigned short   ss_h;

    unsigned short   ds;
    unsigned short   ds_h;

    unsigned short   fs;
    unsigned short   fs_h;

    unsigned short   gs;
    unsigned short   gs_h;

    unsigned short   ldt;
    unsigned short   ldt_h;

    unsigned short   trap;
    unsigned short   iomap;

} tss_struct_t;


tss_struct_t tss_tmp, **tss_entries = (tss_struct_t**)&tss_tmp;
uint32_t total_tss = 0;

//Assembly Functions to load descriptors table onto the processor
extern void lgdt(void *);
extern void lidt(void *);
extern void tss_flush();

typedef uint64_t gdt_entry_t;
typedef uint16_t gdt_ptr_t[3];

typedef uint64_t idt_entry_t;
typedef uint16_t idt_ptr_t[3];

uint32_t cs_TallyTbl[] = {0x8, 0x8, 0x1B, 0x1B};
uint32_t ds_TallyTbl[] = {0x10, 0x23, 0x10, 0x23};

void init_descriptor_tables();

 void SimpleGDT_Setup(uint32_t* gdt, uint32_t* gdtr);
 void General_GDT_Setup(uint32_t* gdt, uint32_t* gdtr);
void SimpleIDT_Setup(uint32_t* idt, uint32_t* idtr);