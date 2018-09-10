#include "phy.h"
#include "math.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "stdlib.h"

/*
	[0-4MB] = KERNEL Binary
	[4-8MB] = KERNEL MAIN PAGE DIRECTORY (sys_dir)
	[8-12MB] = FRAME STACK FOR PHYSICAL FRAME ALLOCATION
	[12-14MB] = MMAD Structures
	[14-16MB] = RESERVED
	[16-24MB] =
	[24-600MB] = KERNEL Resources
	[700MB - 4GB] = USER Free Space
	REST FREE
*/

void memmap_generator()
{
	MemRegion_t *OS_MMap = (MemRegion_t *)((KERNEL_BASE) + (16 * 1024 * 1024));
	Fmemmap = OS_MMap;
	OS_MMap->startHi = 0x0;
	OS_MMap->sizeHi = 0x400000; //4*1024*1024;
	OS_MMap->type = 7;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0x400000; // 4*1024*1024;
	OS_MMap->sizeHi = 0x400000;  //4*1024*1024;
	OS_MMap->type = 8;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0x800000; //8*1024*1024;
	OS_MMap->sizeHi = 0x600000;  //6*1024*1024;
	OS_MMap->type = 9;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0xE00000; //14*1024*1024;
	OS_MMap->sizeHi = 0x200000;  //2*1024*1024;
	OS_MMap->type = 10;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0x1000000; //16*1024*1024;
	OS_MMap->sizeHi = 0x800000;   //8*1024*1024;
	OS_MMap->type = 9;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0x1800000; //24*1024*1024;
	OS_MMap->sizeHi = 0xbd800000; //	 24th mb to 0xBF000000
	OS_MMap->type = 1;
	OS_MMap->reservedt = 0xFFE42;
	//++OS_MMap;
	/*
	++OS_MMap;
	OS_MMap->startHi = (3*1024*1024*1024) + (256*1024*1024);
	OS_MMap->sizeHi = 256*1024*1024;
	OS_MMap->type = 2;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = (3*1024*1024*1024) + (512*1024*1024);
	OS_MMap->sizeHi =(256*1024*1024);
	OS_MMap->type = 2;
	OS_MMap->reservedt = 0xFFE42;*/

	/*************

	OS_MMap->startHi = 0x1E00000;//30*1024*1024;
	OS_MMap->sizeHi = 0xAA00000;//170*1024*1025;			//0x23A00000;//570*1024*1024;
	OS_MMap->type = 11;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 190*1024*1024;
	OS_MMap->sizeHi = 410*1024*1024;
	OS_MMap->type = 1;
	OS_MMap->reservedt = 0xFFE42;
	++OS_MMap;
	OS_MMap->startHi = 0xC800000;//200*1024*1024			//0x2BC00000;//(700*1024*1024);
	OS_MMap->sizeHi = 0x25800000;//600*1024*1024			//0x6300000;//(100*1024*1024);//(2*1024*1024*1024) - (600*1024*1024);
	OS_MMap->type = 1;
	OS_MMap->reservedt = 0xFFE42;

	++OS_MMap;
	OS_MMap->startHi = 0x32000000;//(800*1024*1024);
	OS_MMap->sizeHi = 0x8E000000;//(2272*1024);//(3*1024*1024*1024) - (800*1024*1024);
	//OS_MMap->sizeHi *= 1024;
	OS_MMap->type = 1;
	OS_MMap->reservedt = 0xFFE42;
	*************/

	printf("\nOS Specific Memory Regions Preallocated!");
}

void __attribute__((optimize("O2"))) setup_frameStack()
{
	printf("\nSetting up Frame Stack!");
	uint32_t *frame_stack_ptr = (uint32_t *)(KERNEL_BASE + 0x800000), *frame_stack_start = (uint32_t *)(KERNEL_BASE + 0x800000);
	MemRegion_t *mm = Fmemmap;
	++mm;
	while (1)
	{
		if (mm->type == 1 || mm->type == 12)
		{
			for (uint32_t i = mm->startHi; i < mm->startHi + mm->sizeHi; i += 4096)
			{
				*frame_stack_ptr = i / 4096;
				++frame_stack_ptr;
			}
		}
		++mm;
		if (mm && mm->reservedt != 0xFFE42)
			break;
	}
	frame_stack_end = frame_stack_ptr;
	--frame_stack_end;
	uint32_t size = frame_stack_end - frame_stack_start;
	printf("\nSize of Frame Stack = %d, start : %x, end : %x", size, frame_stack_start, frame_stack_end);
	for (uint32_t i = 0; i < size / 2; i++)
	{
		uint32_t c = frame_stack_start[i];
		frame_stack_start[i] = frame_stack_start[size - i - 1];
		frame_stack_start[size - i - 1] = c;
	}

	//for(int i = 0; i < 500; i++)
	//	printf("%x \t", phy_alloc4K());
}

uint32_t pop_frameStack()
{
	uint32_t fr;
back:
	fr = *frame_stack_end;
	--frame_stack_end;
	//printf("--A--");
	if (!fr)
	{
		printf("\nNo memory left! %x", fr);
		//while(1);
		asm volatile("hlt");
		goto back;
	}
	return fr;
}

void push_frameStack(uint32_t fr)
{
	++frame_stack_end;
	*frame_stack_end = fr;
}

uint32_t phy_alloc4K()
{
	uint32_t addr;
	addr = pop_frameStack() * 4096;
	/*
	if((*(uint32_t*)(get_pageEntry(addr))) & (uint32_t)CUSTOM_PTE_AVAIL_1)	goto back;
	*((uint32_t*)(get_pageEntry(addr))) |= CUSTOM_PTE_AVAIL_1 | CUSTOM_PTE_AVAIL_2;*/
	return addr;
}

/**********************************************/ // Physical Memory Allocator, Kmalloc ///**********************************************/

/*
	We shall have an Index table with indexes of lists of chunks of sizes falling between the sequence -->
	There shall be one continuous long ordered list of free chunks, ordered by size, with specific size groups indexed in the index table.
	There shall be another continous long ordered list for used chunks, ordered by address.

	Each chunk contains this data -> size, start address, end address.

	Each ordered List would reside in consecutive virtual memory, implemented by singly linked list.
	Each chunk would be allocated from a chunk stack (Which fills whenever a chunk not from the end is freed), or from the end of the chunks list memory wise.

	Index Table Entries -->
	32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 ... 2^32
	i.e,
	2^5, 2^6, 2^7, 2^8 ... 2^32 =  32-5 + 1 = 28 entries.

	entries would contain chunks of sizes bigger then or equal to the size of the entry.

	KERNEL Free/Used Lists and other meta is stored at 0xC0C00000; Kernel only mapped
	For any other user process, it is mapped at 0xC1400000;
*/

MemDesc_t *create_mem_desc(uintptr_t virtual_address, uintptr_t start, uintptr_t end) // By default, 11 pages shall by available for each list, so assume, virtual_address + 6 pages are available
{
	/*
		virtual_address = virtual address at which this memory descriptor would be stored
		start = start of the available memory
		end = end of the available memory
	*/
	/*
		Architecture ->
			**page start**
			/Memory Descriptor/
			/Free Ordered List/
			/Used Ordered List/
			/Index Table/
			/chunk stack/
			.
			.
			.
			**page end**
	*/
	size_t size = (size_t)(end - start);

	uintptr_t lpage = virtual_address;
	MemDesc_t *mm = (MemDesc_t *)lpage;
	OrderedList_t *lf = (OrderedList_t *)&(mm->freeList);
	lpage += sizeof(MemDesc_t);
	lf->indexTable = lpage;

	// No upper bounds on chunks list length, assuming they don't cross 10 pages
	// Lets create the default Free chunk

	uintptr_t cpage = virtual_address + (4096 * 2);
	Chunk_t *cc = (Chunk_t *)cpage;

	cc->size = end - start;
	cc->start = start; // For kernel these should be different, change in the outside context
	cc->end = end;
	cc->next = NULL;
	cc->back = NULL;

	// Initializing the Free Index Table...

	IndexTableEntry_t *ent = (IndexTableEntry_t *)lpage;

	for (int i = 31; i >= 0; i--)
	{
		ent[i].index = (size_t)(powf(2, (float)i) - 1);
		//printf("\t[%x, %d]", ent->size, i);
		ent[i].count = 0;
		ent[i].start = cc; // At Initialization, because there is only one big chunk
		ent[i].end = cc;
	}
	uint32_t log_sz = (uint32_t)logx(size, 2); // Get the power of two from which it is greater.
	uint32_t idx = 31 - log_sz;
	cc->IndexTableEntry = (uintptr_t)&ent[idx];
	ent[idx].count = 1;

	mm->available = size;
	mm->used = 0;
	mm->chunk_stack_start = (uintptr_t)&ent[32];//virtual_address + 4096;
	mm->chunk_stack_end = (uintptr_t)&ent[32];//virtual_address + 4096;
	mm->chunk_stack_count = 0;
	mm->chunk_list_terminate = cpage + sizeof(Chunk_t);

	lf->start = cc;
	lf->end = cc;
	lf->chunk_count = 1;

	OrderedList_t *lu = &(mm->usedList);
	lu->indexTable = (uintptr_t)NULL;
	lu->start = NULL;
	lu->end = NULL;
	lu->chunk_count = 0;

	return mm;
}

int f_list_add_chunk(OrderedList_t *list, Chunk_t *chunk)
{
	//printf("[f_list_add_chunk]->");
	//list_print(list);
	size_t size = chunk->size;
	++list->chunk_count;
	IndexTableEntry_t *ent = (IndexTableEntry_t *)list->indexTable;
	uint32_t log_sz = (uint32_t)logx(size, 2); // Get the power of two from which it is greater.
	uint32_t idx = 31 - log_sz;
	Chunk_t *c = ent[idx].start;
	//printf("{{%x %x}}", c, ent);
	//printf("[{%x, %x}]", size, idx);
	if (ent[idx].count) //(&ent[idx] == (IndexTableEntry_t *)c->IndexTableEntry) // if this index entry IS NOT EMPTY
	{
		//printf("{A}");
		chunk->IndexTableEntry = (uintptr_t)&ent[idx];
		int cf = 0; // Flag
		for (size_t i = 0; i < ent[idx].count; i++)
		{
			//printf("(%x)", c->size);
			if (c->size <= size)
			{
				//printf("(%x, %x)", c->size, size);
				chunk->next = c;
				chunk->back = c->back;
				if (c->back)
					c->back->next = chunk;
				c->back = chunk;
				if (i == 0) // Is the largest chunk
				{
					ent[idx].start = chunk;
					//ent[idx].end = ;
				}
				++ent[idx].count;
				cf = 1;
				break;
			}
			c = c->next;
		}
		if (!cf) // i.e, it reached the end of the chunk list but still didnt find anything; This is the smallest
		{
			//printf("{B}");
			c = ent[idx].end;
			ent[idx].end = chunk;
			chunk->back = c;
			chunk->next = c->next;
			if (c->next)
				c->next->back = chunk;
			c->next = chunk;
			++ent[idx].count;
		}
	}
	else // This index entry IS EMPTY
	{
		//printf("{C}");
		//IndexTableEntry_t* ent2 = (IndexTableEntry_t*)c->IndexTableEntry;
		// Basically, add this chunk at the end of the chunk 'c'
		// and create an entry at this index

		// First find this chunk 'c' which is the last chunk with size for than this index.
		for (int i = idx + 1; i >= 0; i--)
		{
			if (ent[i].count)
			{
				c = ent[i].end;
				break;
			}
		}
		if (c)
		{
			//printf("--");
			chunk->next = c->next;
			if (c->next)
				c->next->back = chunk;
			c->next = chunk;
		}
		else // This chunk is the largest; all rest below this
		{
			chunk->next = list->start;
			if (list->start)
				list->start->back = chunk;
			list->start = chunk;
		}
		chunk->back = c;
		chunk->IndexTableEntry = (uintptr_t)&ent[idx];

		ent[idx].start = chunk;
		ent[idx].end = chunk;
		ent[idx].count = 1;
	}
	if (chunk->back == list->end)
	{
		list->end = chunk;
	}
	if (chunk->next == list->start)
	{
		list->start = chunk;
	}
	return 0;
}

int f_list_remove_chunk(OrderedList_t *list, Chunk_t *chunk)
{
	//printf("[f_list_remove_chunk]->");
	IndexTableEntry_t *ent = (IndexTableEntry_t *)chunk->IndexTableEntry;
	//printf("(((%x %x, %x)))", chunk, chunk->size, ent);
	if (chunk->next)
		chunk->next->back = chunk->back;
	else // Obviously this node was the end of the list
		list->end = chunk->back;
	if (chunk->back)
		chunk->back->next = chunk->next;
	else // Obviously this was the start of the list
		list->start = chunk->next;

	--list->chunk_count;
	--ent->count;
	if (ent->count == 0)
	{
		ent->start = chunk->back;
		ent->end = chunk->back;
	}
	else if (chunk == ent->start)
	{
		ent->start = chunk->next;
	}
	else if (chunk == ent->end)
	{
		ent->end = chunk->back;
	}
	return 0;
}

int u_list_add_chunk(OrderedList_t *list, Chunk_t *c)
{
	//printf("[u_list_add_chunk]->");
	//list_print(list);
	/*
		TODO: Add proper algorithm, replace this work arround
	*/
	if (list->chunk_count)
	{
		c->back = list->end;
		list->end->next = c;
		list->end = c;
		c->next = NULL;
	}
	else
	{
		c->next = NULL;
		c->back = NULL;
		list->start = c;
		list->end = c;
	}
	++list->chunk_count;
	return 0;
}

int u_list_remove_chunk(OrderedList_t *list, Chunk_t *chunk)
{
	//printf("[u_list_remove_chunk]->");
	//list_print(list);
	if (chunk->next)
		chunk->next->back = chunk->back;
	if (chunk->back)
		chunk->back->next = chunk->next;
	--list->chunk_count;
	if (chunk == list->end)
	{
		list->end = chunk->back;
	}
	if (chunk == list->start)
	{
		list->start = chunk->next;
	}
	//list_print(list);
	/*
		In case We make an Index table for used Chunks as well in the future, 
		Put other operations here
	*/
	return 0;
}

Chunk_t *u_list_search_chunk_waddr(OrderedList_t *list, uintptr_t addr)
{
	//printf("[u_list_search_chunk_waddr]->");
	Chunk_t *chunk = list->start;
	for (int i = 0; chunk != NULL; i++)
	{
		if (addr >= chunk->start && addr < chunk->end)
		{
			return chunk;
		}
		chunk = chunk->next;
	}
	return NULL;
}

Chunk_t *mem_alloc_chunk(MemDesc_t *mm)
{
	//printf("[mem_alloc_chunk]->");
	Chunk_t *c;
	if (mm->chunk_stack_count == 0)
	{
		c = (Chunk_t *)mm->chunk_list_terminate;
		mm->chunk_list_terminate += sizeof(Chunk_t);
	}
	else
	{
		mm->chunk_stack_end -= sizeof(Chunk_t);
		c = (Chunk_t *)mm->chunk_stack_end;
		--mm->chunk_stack_count;
	}
	memset((void*)c, 0, sizeof(Chunk_t));
	return c;
}

int mem_dealloc_chunk(MemDesc_t *mm, Chunk_t *chunk)
{
	//printf("[mem_dealloc_chunk]->");
	if ((uintptr_t)chunk != mm->chunk_list_terminate)
	{
		++mm->chunk_stack_count;
		mm->chunk_stack_end += sizeof(Chunk_t);
	}
	else
	{
		mm->chunk_list_terminate -= sizeof(Chunk_t);
	}
	memset((void*)chunk, 0, sizeof(Chunk_t));
	return 0;
}

Chunk_t *chunk_split_size(MemDesc_t *mm, Chunk_t *chunk, size_t size)
{
	//printf("[chunk_split_size]->");
	chunk->size -= size;

	Chunk_t *cnew = mem_alloc_chunk(mm);
	cnew->size = size;
	cnew->start = chunk->start;
	chunk->start += size;
	cnew->end = chunk->start;

	return cnew;
}

uintptr_t mem_alloc_memory(MemDesc_t *mm, size_t size)
{
	OrderedList_t *flist = &mm->freeList;
	OrderedList_t *ulist = &mm->usedList;
	//printf("[flist]->");
	//list_print(flist);
	//printf("[ulist]->");
	//list_print(ulist);
	IndexTableEntry_t *ent = (IndexTableEntry_t *)flist->indexTable;
	uint32_t log_sz = (uint32_t)logx(size, 2); // Get the power of two from which it is greater.
	uint32_t idx = 31 - log_sz;
	Chunk_t *c = ent[idx].start;
	//printf(":::%x:::", size);
	int cf = 0; // Flag
	for (size_t i = 0; c != NULL; i++)
	{
		//printf("[K %x]", c->size);
		if (c->size <= size) // Search for the best fit chunk
		{
			//printf("[M]{{%x}}", c->back);
			Chunk_t *cfree = c->back;
			Chunk_t *cused = chunk_split_size(mm, cfree, size);
			f_list_remove_chunk(flist, cfree);
			if (cfree->size != 0)
			{
				f_list_add_chunk(flist, cfree); // This should place this reduced chunk in its appropriate place.
			}
			else
			{
				mem_dealloc_chunk(mm, cfree);
			}
			u_list_add_chunk(ulist, cused);
			return cused->start;
		}
		c = c->next;
	}
	if (!cf) // no smaller chunks exist? Okay, split the last biggest
	{
		//printf("[L]");
		Chunk_t *cfree = flist->end;
		Chunk_t *cused = chunk_split_size(mm, cfree, size);
		//printf("***%x %x %x***", cfree->size, flist->end, ent->start);
		//list_print(flist);
		f_list_remove_chunk(flist, cfree);
		//printf("***%x %x %x***", cfree->size, flist->end->size, ent->start);
		//printf("***%x***", ent->start);
		if (cfree->size != 0)
		{
			f_list_add_chunk(flist, cfree); // This should place this reduced chunk in its appropriate place.
		}
		else
		{
			mem_dealloc_chunk(mm, cfree);
		}
		u_list_add_chunk(ulist, cused);
		return cused->start;
	}
	return 0;
}

int mem_dealloc_memory(MemDesc_t *mm, uintptr_t addr)
{
	//printf("[mem_dealloc_memory]->");
	Chunk_t *c = u_list_search_chunk_waddr(&(mm->usedList), addr);
	if (c == NULL)
	{
		printf("\n[Deallocation Error, Couldn't find addr %x in used list]", addr);
		return 1;
	}
	u_list_remove_chunk(&(mm->usedList), c);
	f_list_add_chunk(&(mm->freeList), c);
	return 0;
}

void* kernel_alloc(size_t size)
{
	return (void*)mem_alloc_memory(KERNEL_MEMDESC, size);
}

int kernel_dealloc(void* addr)
{
	return mem_dealloc_memory(KERNEL_MEMDESC, (uintptr_t)addr);
}

uint32_t pmem_4k(int size)  // Returns 4 Kilobyte aligned Memory i.e, memory address rounded up to 4096.
{
	uint32_t tmp = ((uint32_t)kmalloc((size+1)*4096));
	return ROUNDUP(tmp, 4096);
}

uintptr_t iden_alloc(size_t size)
{
	return (uintptr_t)mem_alloc_memory(KERNEL_IDEN_MEMDESC, size);
}

int iden_dealloc(uintptr_t addr)
{
	return mem_dealloc_memory(KERNEL_IDEN_MEMDESC, addr);
}

int list_print(OrderedList_t *list)
{
	printf("[list_print %x]->", list->chunk_count);
	Chunk_t *c = list->start;
	for (size_t i = 0; i < list->chunk_count; i++)
	{
		if (c->back == NULL)
		{
			printf("[!!!]");
		}
		printf("[<(%x)>]=>", c->size);
		c = c->next;
	}
	return 0;
}

int init_phy_mem()
{
	printf("\nCreating Kernel Memory Descriptor...");
	KERNEL_MEMDESC = create_mem_desc(0xC0C00000, 0xC8400000, 0xFF000000);	// Memory Descriptors for the Kernel, Useful memory from 0xc8400000 to 0xFF000000
	KERNEL_IDEN_MEMDESC = create_mem_desc(0xC0C0F000, 0xBF000000, 0xBFF00000); // 15mb //0xC2000000, 0xC8400000); // For Identity mapped 100 mb
	printf(" %g[Done]%g Kernel Memory : %x\n", 2, 15, KERNEL_MEMDESC->freeList.start->size);

	kmalloc = kernel_alloc;
	kfree = kernel_dealloc;
/*
	// Testing...
	uintptr_t x;
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x4000);
	printf("\n{%x}\t", x);
	//mem_dealloc_memory(KERNEL_MEMDESC, x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x4000);
	printf("\n[%x]\t", x);
	mem_dealloc_memory(KERNEL_MEMDESC, x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x4000);
	printf("\n[%x]\t", x);
	//mem_dealloc_memory(KERNEL_MEMDESC, x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x40000);
	printf("\n[%x]\t", x);
	mem_dealloc_memory(KERNEL_MEMDESC, x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x50000);
	printf("\n[%x]\t", x);
	mem_dealloc_memory(KERNEL_MEMDESC, x);
	printf("\n");
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);
	x = mem_alloc_memory(KERNEL_MEMDESC, 0x1000);
	printf("\n[%x]\t", x);

	printf("\nPrinting the complete Free List...");
	list_print(&(KERNEL_MEMDESC->freeList));
*/
	null_read_4byte = mem_alloc_memory(KERNEL_MEMDESC, 0x8);
	return 0;
}