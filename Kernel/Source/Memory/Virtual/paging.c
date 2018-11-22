#include "paging.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

void Setup_SystemDir()
{
    //system_pdirCap = (Pdir_Capsule_t*)0x400000;
    PageDirectory_t *dir = (PageDirectory_t *)(system_dir);

    // Identity mapping, Page Table for 4-8mb
    dir->table_entry[1] = (table_t)(0x1007); // FRAME | 0x7, 0x7 -> Default Flags
    PageTable_t *entry = (PageTable_t *)(0x1000);
    for (int i = 0; i < 1024; i++)
    {
        entry->page_entry[i] = (page_t)(0x400007 + (i * 0x1000));
    }   
    entry = (PageTable_t*)(0x2000);
    for (int i = 0; i < 1024; i++)
    {
        entry->page_entry[i] = (page_t)(0x000007 + (i * 0x1000));
    }

    dir->table_entry[0] = (table_t)(0x2007);

    // We would map first 20mb to the higher half

    for (int i = 0; i < 5; i++)
    {
        // The Page Tables for the mappings of first 20mb of 3rd GB would reside in the 4mb to 8mb region, with an offset of (table_index + 0x300) * 4096
        dir->table_entry[i + 0x300] = (table_t)(0x400007 + ((i + 0x300) * 0x1000));
        entry = (PageTable_t *)(0x400000 + ((i + 0x300) * 0x1000));
        for (int j = 0; j < 1024; j++)
        {
            entry->page_entry[j] = (page_t)((0x400000 * i) + 0x7 + (j * 0x1000));
        }
    }

    for (int i = 2; i < 1024; i++)
    {
        dir->table_entry[i] = (table_t)(0x400007 + (i * 0x1000));
    }
    //dir->table_entry[1] = 0xC0401007;
    // Now we create default page tables for every page
    printf("\n[%x] [%x]", *(uint32_t*)0xC0701000, *(uint32_t*)0x701000);
   // memset((void *)0x1000, 0xff, 2048); // We no more need the identity mapped 4th to 8th mb.
/*
    // For Testing
    *(uint32_t*)(0xC0100000) = 0;
    *(uint32_t*)(0xC0400000) = 0;
    *(uint32_t*)(0xC0800000) = 0;
    *(uint32_t*)(0xC0C00000) = 0;
    *(uint32_t*)(0xC0410000) = 0;
    *(uint32_t*)(0x00412300) = 0;
    *(uint32_t*)(0xC0100000) = 0;*/
}

uintptr_t pgdir_create(uint32_t pgdir)        // General Page Directories
{   
    // First allocate enough memory to save the page directory structure
    PageDirectory_t *dir;
    if(pgdir == NULL)
        dir = phy_alloc4K();
    else 
        dir = pgdir;

    // Map the kernel to this new page directory, the 3rd GB and above.
    //Map_identity_kernelOnly(0xBF000000, 0xFFFFF000-0xBF000000, dir); 
    PageDirectory_t* tmp = system_dir;
    // Identity map first mb because GDT and IDT reside there. Have to change this
    for (int i = 0; i < 0; i++)
    {
        dir->table_entry[i] = system_dir->table_entry[i];
    }
    for (int i = 764; i < 1024; i++)
    {
        dir->table_entry[i] = system_dir->table_entry[i];
    }   
    return dir;
}

void Map_non_identity(uint32_t phys, uint32_t virt, uint32_t size, PageDirectory_t *dir)
{
    for (uint32_t i = phys, j = virt; i < phys + size; i += 4096, j += 4096)
    {
        //printf(" %x",i);
        page_t *page;
        page = get_page(j, 1, dir); //kernel Pages;
        pt_entry_set_frame(page, i);
        pt_entry_add_attrib(page, I86_PTE_PRESENT);
        pt_entry_add_attrib(page, I86_PTE_WRITABLE);
        pt_entry_add_attrib(page, I86_PTE_USER);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

void Unmap_non_identity(uint32_t phys, uint32_t virt, uint32_t size, PageDirectory_t *dir)
{
    for (uint32_t i = phys, j = virt; i < phys + size; i += 4096, j += 4096)
    {
        //printf(" %x",i);
        page_t *page;
        page = get_page(j, 1, dir); //kernel Pages;
        pt_entry_set_frame(page, 0);
        pt_entry_del_attrib(page, I86_PTE_PRESENT);
        pt_entry_del_attrib(page, I86_PTE_WRITABLE);
        pt_entry_del_attrib(page, I86_PTE_USER);
        pt_entry_del_attrib(page, CUSTOM_PTE_AVAIL_1);
        pt_entry_del_attrib(page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

void Create_PTable(uint32_t phy, PageTable_t *tbl)
{
    for (uint32_t i = phy, k = 0; k < 1024; i += 4096, k++)
    {
        //printf(" %x",i);
        page_t *page = &tbl->page_entry[k];
        pt_entry_set_frame(page, i);
        pt_entry_add_attrib(page, I86_PTE_PRESENT);
        pt_entry_add_attrib(page, I86_PTE_WRITABLE);
        pt_entry_add_attrib(page, I86_PTE_USER);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

void flush_tlb_entry(uint32_t addr)
{
    asm volatile("cli");
    asm volatile("invlpg (%0)" ::"r"(addr)
                 : "memory");
    asm volatile("sti");
}

page_t *MapPage(void *phys, void *virt, PageDirectory_t *dir)
{
    //! get page directory
    PageDirectory_t *pageDirectory = dir;

    //! get page table
    table_t *e = &pageDirectory->table_entry[PAGE_DIRECTORY_INDEX((uint32_t)virt)];
    if (!*e)
    {
        //! page table not present, allocate it
        PageTable_t *table = (PageTable_t *)phy_alloc4K();
        if (!table)
            return 0;

        //! clear page table
        //memset (table, 0, sizeof(PageTable_t));

        //! create a new entry
        table_t *entry = &pageDirectory->table_entry[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

        //! map in the table (Can also just do *entry |= 3) to enable these bits
        pd_entry_add_attrib(entry, I86_PDE_PRESENT);
        pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
        pd_entry_add_attrib(entry, I86_PDE_USER);
        pd_entry_set_frame(entry, (uint32_t)table);
    }

    //! get table
    PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(e);

    //! get page
    page_t *page = &table->page_entry[PAGE_TABLE_INDEX((uint32_t)virt)];

    //! map it in (Can also do (*page |= 3 to enable..)
    pt_entry_set_frame(page, (uint32_t)phys);
    pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_1);
    pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_2);
    pt_entry_add_attrib(page, I86_PTE_PRESENT);
    pt_entry_add_attrib(page, I86_PTE_USER);
    pt_entry_add_attrib(page, I86_PTE_WRITABLE);
    return page;
}

void Map_identity(uint32_t phy, size_t size, PageDirectory_t *dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j = phy;
    for (; j < phy + size; j += 0x1000)
    {
        //MapPage((void*)j,(void*)j,dir);
        page_t *page;
        page = get_page(j, 1, dir);
        pt_entry_set_frame(page, j);
        pt_entry_add_attrib(page, I86_PTE_PRESENT);
        pt_entry_add_attrib(page, I86_PTE_WRITABLE);
        pt_entry_add_attrib(page, I86_PTE_USER);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_2);
    }
}

void Map_identity_readOnly(uint32_t phy, size_t size, PageDirectory_t *dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j = phy;
    for (; j < phy + size; j += 0x1000)
    {
        //MapPage((void*)j,(void*)j,dir);
        page_t *page;
        page = get_page(j, 1, dir);
        pt_entry_set_frame(page, j);
        pt_entry_add_attrib(page, I86_PTE_PRESENT);
        // pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
        pt_entry_add_attrib(page, I86_PTE_USER);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib(page, CUSTOM_PTE_AVAIL_2);
    }
}

void Map_identity_kernelOnly(uint32_t phy, size_t size, PageDirectory_t *dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j = phy;
    for (; j < phy + size; j += 0x1000)
    {
        //MapPage((void*)j,(void*)j,dir);
        page_t *page;
        page = get_page(j, 1, dir);
        //printf("[%x]", page);
        pt_entry_set_frame ( page, j);
        pt_entry_add_attrib ( page, I86_PTE_PRESENT);
        pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
        // pt_entry_add_attrib ( page, I86_PTE_USER);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
    }
}

page_t *get_page(uint32_t addr, int make, PageDirectory_t *dir)
{

    // Turn the address into an index.
    addr /= 0x1000;
    // Find the page table containing this address.
    uint32_t table_idx = addr / 1024;
    // printf("\ntable_idx : %x, %x", table_idx, dir->table_entry[0x1]);
    //  while(1);
    if (dir->table_entry[table_idx]) // If this table is already assigned
    {
        //printf("1 ");
        table_t *entry = &dir->table_entry[table_idx];
        //printf("\n[%x]", entry);
        PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(entry);
        return &table->page_entry[addr % 1024];
    }
    else if (make)
    {
        dir->table_entry[table_idx] = (table_t)phy_alloc4K();

        //  map(dir->table_entry[table_idx], 4096, dir);

        //memset_fast((void*)dir->table_entry[table_idx], 0, 0x1000);
        table_t *entry = &dir->table_entry[table_idx];
        PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(entry);
        pd_entry_add_attrib(entry, I86_PDE_PRESENT);
        pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
        pd_entry_add_attrib(entry, I86_PDE_USER);
        //pd_entry_set_frame (entry, (uint32_t)dir->m_entries[table_idx]);
        //  dir->tables[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        return &table->page_entry[addr % 1024];
    }
    else
    {
        printf(" Cant get the Page! ");
        return 0;
    }
}

page_t *get_pageEntry(uint32_t addr)
{
    uint32_t frame = addr / 4096;
    uint32_t pd_off = frame / 1024;
    return &((PageTable_t *)(PAGE_DIRECTORY_INDEX(_cur_dir->table_entry[pd_off])))->page_entry[frame % 1024];
}

table_t *get_tableEntry(uint32_t addr)
{
    uint32_t frame = addr / 4096;
    return &_cur_dir->table_entry[frame / 1024];
}

PageTable_t *get_table(uint32_t addr)
{
    uint32_t frame = addr / 4096;
    uint32_t pd_off = frame / 1024;
    return (PageTable_t *)(PAGE_DIRECTORY_INDEX(_cur_dir->table_entry[pd_off]));
}

uint32_t __attribute__((optimize("O2"))) get_phyAddr(uint32_t addr, PageDirectory_t *dir)
{
    uint32_t tdrr = addr;
    addr /= 0x1000;
    // Find the page table containing this address.

    table_t *entry = &dir->table_entry[addr / 1024];
    PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(entry);

    uint32_t frame = ROUNDDOWN(table->page_entry[addr % 1024], 4096);
    frame += tdrr % 4096;
    return frame;
}


void switch_directory(PageDirectory_t *dir)
{
  //Get_Scheduler()->current_pdir = (uint32_t*)dir;
    asm volatile("mov %0, %%cr3":: "r"((uint32_t)dir):"memory");
}

