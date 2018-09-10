#include "Physical/phy.c"
#include "Virtual/virt.c"
#include "Virtual/paging.c"
#include "stdlib.h"

/*
    Memory Map -->

    0-4mb   ->  Kernel Binary + other stuff
    4-8mb   ->  Page Table entries and stuff
    8-10mb  ->  Memory Page Frame stack
    10-12   ->  free
    12-14   ->  Kernel Memory Descriptor, first 10 pages.   
        0xC00000 - 0xC0a000  -> Kernel Memory Descriptor
        0xC0a000 - 0xC0b000  -> interrupts_tmp_esp
    14-16   ->  reserved
    16-20   ->  Memory Map + other stuff, Lots of space here also
    20+     ->  available

    0xBF000000 - 0xBFF00000 -> identity mapped 15 MB
    0xC0000000  above-->
    same mapping for first 20mb.
   ;// 0xC2000000 to 0xC2000000 + 100 MB -> Identity Memory Mapped Region
*/

int init_basic_mem()
{
    printf("\nInitializing some basic important memory things...");
    printf("\n\tRetrieving System Page Directory Address...");
    asm volatile("mov %%cr3, %%eax; mov %%eax, %0":"=r"(system_dir));
    printf(" %g[Done]%g", 2, 15);

    printf("\n\tSetting up kernel page directory...");
    Setup_SystemDir();
    printf(" %g[Done]%g", 2, 15);

    printf("\n\tSetting up physical memory frame stack...");
    memmap_generator();
    setup_frameStack();
    phy_alloc4K();
    interrupts_tmp_esp = 0xc0c0b000;    // Just after the Kernel Memory Descriptor.
    printf(" %g[Done]%g", 2, 15);
    
    printf("\n\tMapping few important page tables...");
    Map_identity_kernelOnly(0xF0000000, 0xFFFFF000-0xF0000000, system_dir); 
    Map_identity_kernelOnly(0xBF000000, 0xBFFFF000-0xBF000000, system_dir); // 15 MB of memory identity mapped for other purposes //(0xC2000000, 0x6400000, system_dir); // 0xC2000000 100mb of memory mapped region for ahci data transfer usage
    //Map_identity_kernelOnly(0xBFF00000, 0xBFFFF000-0xBFF00000, system_dir); 
    printf(" %g[Done]%g", 2, 15);
    return 0;
}

int init_mem()
{
    init_phy_mem();
    return 0;
}