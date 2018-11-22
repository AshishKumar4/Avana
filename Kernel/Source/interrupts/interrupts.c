#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Memory/Virtual/paging.h"
#include "Memory/Physical/phy.h"

void defaultExceptionHandle()
{
}

void defaultInterruptHandle()
{
}

extern uint32_t pfault_cr2;
extern uint32_t pfault_cr3;

void pageFaultHandle()
{
    //while(1);
   // printf("\n<pfault>\n");
    uint32_t addr = pfault_cr2;
    addr /= 0x1000;
    uint32_t table_idx = addr / 1024;
    PageDirectory_t *dir = (PageDirectory_t *)pfault_cr3;
    page_t *page;

    if (dir->table_entry[table_idx]) // If this table is already assigned
    {
        table_t *entry = &dir->table_entry[table_idx];
        PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(entry);
        page = &table->page_entry[addr % 1024];
    }
    else
    {
        printf("\nASDASD");
        dir->table_entry[table_idx] = (table_t)phy_alloc4K();
        // memset_fast((void *)dir->table_entry[table_idx], 0, 0x1000);
        table_t *entry = &dir->table_entry[table_idx];
        PageTable_t *table = (PageTable_t *)PAGE_GET_PHYSICAL_ADDRESS(entry);
        *entry |= I86_PDE_PRESENT;
        *entry |= I86_PDE_WRITABLE;
        *entry |= I86_PDE_USER;
        //pd_entry_set_frame (entry, (uint32_t)dir->m_entries[table_idx]);
        //  dir->tables[table_idx] = tmp | 0x7; // PRESENT, RW, US.
        page = &table->page_entry[addr % 1024];
    }
    if (*page)
    {
        // Must be a case of Copy on Write
        printf("\nWRITE ERROR!!!!!"); /*
        Shell_Dbuff_sync();
        asm volatile("hlt");//*/
        //printf("[{*%x*}]",*page);
        *page |= I86_PTE_PRESENT;
        *page |= I86_PTE_WRITABLE;
        *page |= I86_PTE_USER;
        return;
    }
    uint32_t phy_frame = pop_frameStack();
    *page = phy_frame * 4096;
    *page |= I86_PTE_PRESENT;
    *page |= I86_PTE_WRITABLE;
    *page |= I86_PTE_USER;

    //printf("\n%x %x %x", get_page(pfault_cr2, 0, dir), page, *page);
}

void PIT_Handle()
{
    printf("\nH");
    //asm volatile("int $51");
}