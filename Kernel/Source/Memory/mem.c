#include "Physical/phy.c"
#include "Virtual/virt.c"
#include "Virtual/paging.c"
#include "stdlib.h"
#include "Processing/tasking.h"

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
*/

//int MEM_DESC_PTR = 0xC0C1e000;
/*
extern SchedulerKits_t;
extern SchedulerKits_t* Get_Scheduler();
*/
uintptr_t Gen_MemStructures()
{
    /*
        Always, The Memory Descriptors for any normal process would be stored just after
        the process's page directory!
    */
	//int a = MEM_DESC_PTR;
	//MEM_DESC_PTR += 0xf000;

    uint32_t a = ROUNDUP(iden_alloc(4092*11), 4096);
    Map_identity_kernelOnly(a, 4096*11, system_dir); // It is just a workaround, I NEED TO REPLACE IT
    memset(a, 0, 4096*11);
    printf("\nMemory At %x, phy: %x", a, get_phyAddr(a, system_dir));

    PageDirectory_t* pgdir;
    pgdir = pgdir_create(a);

	MemDesc_t* mdesc = create_mem_desc((uintptr_t)(a + 4096), 0x2000000, 0xBF000000);
	mdesc->pgdir = pgdir;
	return a;
}

uintptr_t vmalloc(int size)
{
    uint32_t pgdir = Get_Scheduler()->current_pdir;
    PageDirectory_t* dir = (PageDirectory_t*)pgdir;
    MemDesc_t* mm = (MemDesc_t*)(pgdir + 4096);

    // Actually Allocate some memory
    uint32_t baddr = mem_alloc_memory(mm, size);

	//Create necessary page tables and pages for the allocated memory.
	uint32_t pg_frame = baddr/4096;
	uint32_t pd_off = pg_frame/1024;
	uint32_t pt_off = pg_frame%1024;
	if(!dir->table_entry[pd_off])
	{
		//printf("A");
		//Create the table, and then the page.
		table_t* entry = &dir->table_entry[pd_off];
		*entry = phy_alloc4K();
		pd_entry_add_attrib (entry, I86_PDE_PRESENT);
		pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
		pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_1);
		pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_2);
	}
	else
	{
		//printf("B");
		PageTable_t* pt = (PageTable_t*)PAGE_GET_PHYSICAL_ADDRESS(&dir->table_entry[pd_off]);
		//Create the page.
		page_t* pg = &pt->page_entry[pt_off];
		uint32_t i;
		for(i =  pg_frame; i<(size/4096) + pg_frame; i++)
		{
			if(!(i%1024))   //Whenever crossing page table boundries, just switch to next page tables.
			{
				table_t* entry = &dir->table_entry[i/1024];
				if(!entry)
				{
					//printf(" b2");
					*entry = phy_alloc4K();
					pd_entry_add_attrib (entry, I86_PDE_PRESENT);
					pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
					pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_1);
					pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_2);
				}
				pt = (PageTable_t*)PAGE_GET_PHYSICAL_ADDRESS(entry);
				pg = &pt->page_entry[0];
			}
			//printf(" b1");
			*pg = 1027 | CUSTOM_PTE_AVAIL_2 | phy_alloc4K();
			++pg;
		}
	}
	if(size%4096)
	{
		//printf("C");
		PageTable_t* pt = (PageTable_t*)PAGE_GET_PHYSICAL_ADDRESS(&dir->table_entry[pd_off]);
		//Create the page.
		page_t* pg = &pt->page_entry[pt_off];
		if(!*pg)
		{
			//printf(" c1");
			uint32_t phy_mem = phy_alloc4K();
			*pg = 1027 | CUSTOM_PTE_AVAIL_1 | phy_mem;
		}
	//	printf(" pp%d %d", pg, *pg);

		//Make appropriate memory strips.
	}
    return baddr;
}

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
    // Remember to replace below functions by identity mapping for kernel only
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
