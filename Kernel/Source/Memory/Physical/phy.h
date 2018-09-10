#pragma once

#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

#define FRAME_STACK_START 10240*1024 //8th M

uint32_t vga;
uint32_t maxmem=0;
uint32_t memAvailable=0;
uint16_t blockID=10;
uint32_t max_mem = 0;

typedef struct memory_region
{
	uint32_t	startLo;
	uint32_t	startHi;
	uint32_t	sizeLo;
	uint32_t	sizeHi;
	uint32_t	reservedt;
	uint32_t	type;
}MemRegion_t;

MemRegion_t *mmap_info, *Fmemmap;

//! different memory regions (in memory_region.type)
char* strMemoryTypes[] = {

	"Available",				//memory_region.type==0
	"Reserved",					//memory_region.type==1
	"ACPI Reclaim",				//memory_region.type==2
	"ACPI NVS Memory",			//memory_region.type==3
	"Kernel Reserved",			//memory_region.type==4
	"Kernel Page Directory"		//memory_region.type==4
};

uint32_t* frame_stack_end = 0;

typedef struct Chunk 
{
	size_t 			size;
	uintptr_t 		start;
	uintptr_t		end;
	uintptr_t		IndexTableEntry;
	struct Chunk* 	next;
	struct Chunk* 	back;
}Chunk_t;

typedef struct OrderedList 
{
	uintptr_t		indexTable;
	Chunk_t*		start;
	Chunk_t*		end;
	size_t			chunk_count;
}OrderedList_t;

typedef struct MemDesc
{
	uintptr_t		reserved1;
	size_t			used;
	size_t			available;
	uintptr_t 		chunk_stack_start;
	uintptr_t 		chunk_stack_end;
	size_t			chunk_stack_count;
	uintptr_t		chunk_list_terminate;	// Where in the memory does the list reaches at max
	OrderedList_t	freeList;
	OrderedList_t	usedList;
}MemDesc_t;

typedef struct IndexTableEntry
{
	size_t			index;
	size_t			count;
	Chunk_t*		start;
	Chunk_t*		end;
}IndexTableEntry_t;

MemDesc_t* KERNEL_MEMDESC, *KERNEL_IDEN_MEMDESC;

void memmap_generator();

void setup_frameStack();

uint32_t pop_frameStack();

void push_frameStack(uint32_t fr);

uint32_t phy_alloc4K();
