#include "KLibrary.c"
#include "Sources.c"

int kernel_preInit(uintptr_t multiboot_header)
{
    //asm volatile("sti");
    init_console();
    printf("\nGot Multiboot Headers at address %d", multiboot_header);
    init_descriptor_tables();
    init_basic_mem();
    init_mem();

    init_basic_hardware();       // PCI, AHCI, ETC.
    init_cpu_x86();
    vfs_init();//Aqfs2_Checkfs();
    //Aq_ListEntrys_direct(Aq_CurrentDir);
    return 0;
}

int kernel_Init()
{
 /*   init_fs();
    init_basic_threads();
    init_system();

    basic_final_test();*/
    return 1;
}