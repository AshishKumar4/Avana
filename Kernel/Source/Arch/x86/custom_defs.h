#pragma once 

#include "stdint.h"
#include <stdbool.h>

typedef void (*func_t)();	//void function pointer
typedef void (*func1_t)(int);	//void function pointer
typedef uintptr_t (*func_ptr_t)();	//void function pointer
typedef uint32_t (*uintfunc2_t)(uint32_t, uint32_t);	//int function pointer with 2 arguments
typedef int (*intfunc1_t)(int);	//int function pointer with 1 arguments

#ifndef NULL
#define NULL ((void*) 0)
#endif

#define KERNEL_BASE 0xC0000000

//extern uintptr_t BootPageDirectory;
extern uintptr_t interrupts_tmp_esp;

// Efficient min and max operations
#define MIN(_a, _b)						\
({								\
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a <= __b ? __a : __b;					\
})
#define MAX(_a, _b)						\
({								\
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a >= __b ? __a : __b;					\
})

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)						\
({								\
	uint32_t __a = (uint32_t) (a);				\
	(typeof(a)) (__a - __a % (n));				\
})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)						\
({								\
	uint32_t __n = (uint32_t) (n);				\
	(typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));	\
})

#define sgn(x) ((x<0)?-1:((x>0)?1:0)) /* macro to return the sign of a
                                         number */
#define abs(x) ((x<0)?-x:x) /* macro to return the absolute magnitude of a
																				number */


#define DECLARE_LOCK(name) volatile int name ## Locked
#define LOCK(name) \
	while (!__sync_bool_compare_and_swap(& name ## Locked, 0, 1)); \
	__sync_synchronize();
#define UNLOCK(name) \
	__sync_synchronize(); \
	name ## Locked = 0;

DECLARE_LOCK(THREADTABLE_LOCK);
DECLARE_LOCK(TASK_LOCK_KILL);

typedef struct registers
 {
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
 } registers_t;


inline uint32_t Lower32(uint64_t val)
{
	uint32_t a=val&0xffffffff;
	return a;
}

inline uint32_t Higher32(uint64_t val)
{
	uint32_t b=val>>32;
	return b;
}

inline uint32_t Lower16(uint32_t val)
{
	uint32_t a=val&0xffff;
	return a;
}

inline uint32_t Higher16(uint32_t val)
{
	uint32_t b=val>>16;
	return b;
}

#define TRUE    1  //define a few variable
#define ON      1
#define FALSE   0
#define OFF     0


#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

void exit()
{
  asm volatile("cli; hlt");
}

void panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

typedef uint32_t size_t;

uint32_t total_CPU_Cores = 0;
uint32_t BSP_id = 0;

func1_t sleep;

uintptr_t null_read_4byte;


uintptr_t iden_alloc(size_t size);
int iden_dealloc(uintptr_t addr);
uint32_t pmem_4k(int size);  // Returns 4 Kilobyte aligned Memory i.e, memory address rounded up to 4096.

uint32_t StrToInt(char *str)
{
	if(!str) return 0;
    uint32_t in=0;
    int ln=strlen(str);
    int arr[999],a=1;
    for(int i=0;i<=ln;i++) a=a*10;
    a=a/100;
    for(int i=0;i<=ln;i++)
    {
        arr[i]=str[i]-48;
        in=in+arr[i]*a;
        a=a/10;
    }
	return in;
}

int ByteSequence_Replace(uint32_t magic, uint32_t nbytes, uint32_t replacement, uint32_t mbytes, uint32_t* start, uint32_t* end)
{
  uint32_t m = magic;
  uint8_t* tmp = (uint8_t*)&m;
  uint32_t r = replacement;
  uint8_t* tmp2 = (uint8_t*)&r;
  uint8_t* i = (uint8_t*)start;
  uint32_t j = 0;
  uint32_t c = 0;
  if(end-nbytes < start) return -1;
  for(; i<=((uint8_t*)end)-nbytes; i++)
  {
    for(; j<nbytes; j++)
    {
      if(!(*(i+j) == tmp[j])) //Sequence dosent match
      {
        goto out;
      }
    }
    memcpy(i,tmp2,mbytes); //Sequence Matched; Replace it.
    ++c;
    out:
    j = 0;
  }
  return c;
}

int posix_time()
{
  return 0;
}
