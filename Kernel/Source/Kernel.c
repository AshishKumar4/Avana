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

    tss_entries = iden_alloc(sizeof(tss_struct_t)*100);

    init_basic_hardware();       // PCI, AHCI, ETC.
    init_cpu_x86();
    /*Aqfs2_Partitioner(0, 5, 200*1024);
    Aqfs2_burn(0);*/
    vfs_init();

    //Aq_ListEntrys(".");
    init_multitasking();
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