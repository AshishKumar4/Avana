#include "Console_handle.h"
#include "Console/Console.h"

#include "stdlib.h"
#include "stdio.h"
#include "Processing/tasking.h"
#include "../../std_iohandling.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Memory/mem.h"
#include "../../Serial_Processing/serials.h"

void console_io_init()
{
  console_Start_q = 0;
  console_Last_q = 0;
}

void _stdprint(const char* str, uint32_t length)
{
  SchedulerKits_t* kit = Get_Scheduler();
  PageDirectory_t* curr_dir = kit->current_pdir;
  switch_directory(system_dir);
  Serial_output((char*)str, length, CONSOLE_OUT_FLAG, kit->current_task, 0, (serials_o_struct_t **)&console_Start_q, (serials_o_struct_t **)&console_Last_q, &console_q_elements);
  asm volatile("int $50;");
  switch_directory(curr_dir);
}
