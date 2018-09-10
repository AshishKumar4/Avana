#include "stdint.h"
#include "desc.h"
#include "string.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

extern void defaultExceptionHandler();
extern void defaultInterruptHandler();
extern void pageFaultHandler();
extern void PIT_Handler();

uint32_t descMalloc_ptr = 0x00007E00;
uint32_t descMalloc_cnt = 0; 


uintptr_t desc_malloc(uint32_t size)
{
    if(descMalloc_cnt + size >= 4096)
    {
    //descMalloc_ptr = pmem_4k(1);//phy_alloc4K();
    }
    uint32_t a = descMalloc_ptr;
    descMalloc_ptr += size;
    return a;
}

static uint8_t makeFlagByte(int isPresent, int ring)
{
    return (((isPresent & 0x01) << 7) | ((ring & 0x3) << 5) | (0b1110)) & (~(1));
}

// Set the value of one IDT entry.
static void idtSetEntry(int num, uint32_t base, uint32_t sel, uint32_t flags, uint64_t* idt_base)
{
    uint32_t* entry = (uint32_t*)&idt_base[num];
    *entry = (base & 0xFFFF) | (sel << 16);
    *(entry+1) = 0x0000 | (flags << 8) | (((base >> 16) & 0xFFFF) << 16);
}

// Set the value of one GDT entry.
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint16_t gran, uint64_t* gdt_base)
{
    uint32_t* entry = (uint32_t*)&gdt_base[num];
    *entry = (limit & 0xffff) | ((base & 0xffff)<<16); //lower 32 bit
    *(entry+1) = (((base >> 16) & 0xFF)) | ((access & 0xff)<<8) | (((limit >> 16) & 0x0F)<<16) | ((gran & 0xf0)<<16) | (((base >> 24) & 0xFF)<<24);
}


 void SimpleGDT_Setup(uint32_t* gdt, uint32_t* gdtr)
 {
    uint32_t* gdt_new = gdtr;
    uint16_t* gdt_new_ptr = (uint16_t*)gdtr;
    uint64_t* gdt_new_entries = (uint64_t*)gdt;
    memset(gdt_new, 0, 8);
    memset(gdt_new_entries, 0, 40);

    *gdt_new_ptr = ((sizeof(gdt_entry_t) * 5) - 1);
    ++gdt_new_ptr;
    uint32_t* gdt_new_ptr2=(uint32_t*)gdt_new_ptr;
    *gdt_new_ptr2 = ((uint32_t)gdt_new_entries);
    //while(1);
    gdt_set_gate(0, 0, 0, 0, 0, gdt_new_entries);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF, (uint64_t*)gdt_new_entries); // Code segment - 0x08
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF, (uint64_t*)gdt_new_entries); // Data segment  - 0x10

    //  You ain't gonna need User mode segments anytime soon
    //  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF, (uint64_t*)gdt_new_entries); // User mode code segment - 0x18 
    //  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF, (uint64_t*)gdt_new_entries); // User mode data segment  - 0x20

}

 void General_GDT_Setup(uint32_t* gdt, uint32_t* gdtr)
 {
   uint32_t* gdt_new = gdtr;
   uint16_t* gdt_new_ptr = (uint16_t*)gdtr;
   uint64_t* gdt_new_entries = (uint64_t*)gdt;
   memset(gdt_new, 0, 8);
   memset(gdt_new_entries, 0, 40);

   *gdt_new_ptr = ((sizeof(gdt_entry_t) * 6) - 1);
   ++gdt_new_ptr;
   uint32_t* gdt_new_ptr2=(uint32_t*)gdt_new_ptr;
   *gdt_new_ptr2 = ((uint32_t)gdt_new_entries);
   uint32_t tmp = desc_malloc(sizeof(tss_struct_t));
   tss_struct_t* tss = (tss_struct_t*)tmp;
   gdt_set_gate(0, 0, 0, 0, 0, gdt_new_entries);                // Null segment
   gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF, (uint64_t*)gdt_new_entries); // Code segment - 0x08
   gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF, (uint64_t*)gdt_new_entries); // Data segment  - 0x10
   gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF, (uint64_t*)gdt_new_entries); // User mode code segment - 0x18
   gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF, (uint64_t*)gdt_new_entries); // User mode data segment  - 0x20
   gdt_set_gate(5,(uint32_t)tss,sizeof(tss_struct_t),0x89,0x40, (uint64_t*)gdt_new_entries);
   tss->ss0=0x10;
   tmp += 4096;
   tss->esp0=tmp;
   tss->iomap=sizeof(tss_struct_t);
   tss_entries[total_tss] = tss;
   ++total_tss;
 }

/*
    Some sections of the IDT setup below are commented out. The Interrupt handler functions used in the commented sections
    as well as the Fault handlers DOES NOT USE a CONTEXT SAVE, and are instead directly called. You have to manually
    create Context Saving Wrappers for all the interrupts you wish to use just like the ones already provided. Check
    Interrupts.asm
*/

void SimpleIDT_Setup(uint32_t* idt, uint32_t* idtr)
{
    //The limit is 1 less than our table size because this is the end address
    uint16_t* ptr = (uint16_t*)idtr;
    *ptr = ((sizeof(idt_entry_t) * 256) - 1);
    ++ptr;
    uint32_t* ptr2=(uint32_t*)ptr;
    *ptr2 = ((uint32_t)idt);
    //We need to make sure the interrupt values are nulled out, else BAD things may happen
    memset(idt, 0, sizeof(idt_entry_t)*256);

    for(int i = 0; i < 18 ; i++)   // Exception Handlers
    {
        idtSetEntry(i, (uint32_t)&defaultExceptionHandler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    }

    for(int i = 32; i < 47 ; i++)   // Interrupt Handlers
    {
        idtSetEntry(i, (uint32_t)&defaultInterruptHandler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    }

    /************* Below are overrides *************/
    
    /*idtSetEntry(0, (uint32_t)&divByZero_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt); //0
    idtSetEntry(1, (uint32_t)&debug_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(2, (uint32_t)&NMI_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(3, (uint32_t)&breakpoint_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(4, (uint32_t)&overflow_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(5, (uint32_t)&outOfBounds_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(6, (uint32_t)&invalidInstr_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(7, (uint32_t)&noCoprocessor_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(8, (uint32_t)&doubleFault_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(9, (uint32_t)&coprocessor_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(10, (uint32_t)&badTSS_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(11, (uint32_t)&segmentNotPresent_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(12, (uint32_t)&stackFault_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(13, (uint32_t)&generalProtectionFault_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);*/
    idtSetEntry(14, (uint32_t)&pageFaultHandler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    /*idtSetEntry(15, (uint32_t)&unknownInterrupt_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(16, (uint32_t)&coprocessorFault_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(17, (uint32_t)&alignmentCheck_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(18, (uint32_t)&machineCheck_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);*/
    
    idtSetEntry(32, (uint32_t)&PIT_Handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    /*idtSetEntry(33, (uint32_t)&kb_handle, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(34, (uint32_t)&cascade_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(35, (uint32_t)&COM2_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(36, (uint32_t)&COM1_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(37, (uint32_t)&LPT2_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(38, (uint32_t)&floppyDisk_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(39, (uint32_t)&LPT1_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(40, (uint32_t)&RTC_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(41, (uint32_t)&periph1_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(42, (uint32_t)&periph2_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(43, (uint32_t)&periph3_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(44, (uint32_t)&mouse_handle, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(45, (uint32_t)&FPU_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(46, (uint32_t)&primaryHDD_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt);
    idtSetEntry(47, (uint32_t)&secondaryHDD_handler, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt); */
}

/****************************** INITIALIZATION PART *********************************/

/*
    Once Our OS Gets advanced enough, Replace these shitty Static things with Dynamically
    allocated memory regions i.e, use Malloc for these.
*/

volatile gdt_entry_t gdt_entries[10];
volatile gdt_ptr_t   gdt_ptr;
volatile idt_entry_t idt_entries[256];
volatile idt_ptr_t   idt_ptr;

void init_descriptor_tables()
{
    // Initialise the global descriptor table.
    printf("\nInitializing Temporary Global Descriptor Table for BSP...");
    SimpleGDT_Setup((uint32_t*)&gdt_entries, (uint32_t*)gdt_ptr);
    lgdt((void *)&gdt_ptr);
    printf(" %g[Done]%g", 2, 15);

    printf("\nInitializing Temporary Interrupt Descriptor Table for BSP...");
    SimpleIDT_Setup((uint32_t*)&idt_entries, (uint32_t*)idt_ptr);
    lidt((void *)&idt_ptr);
    printf(" %g[Done]%g", 2, 15);
 }