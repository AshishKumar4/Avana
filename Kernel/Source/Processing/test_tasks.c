#include "task.h"
#include "process.h"
#include "ThreadTables/ThreadTable.h"
#include "Scheduler/Scheduler.h"
#include "IPCInterface/IPCInterface.h"
#include "ProcManager/ProcManager.h"
#include "Locks/Spinlocks.h"
#include "FileSystem/vfs.h"


void test_task1()
{
  printf("\n[Test Task 1] Started, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  printf("\n%d", Get_Scheduler()->identity);
  delay1(1); asm volatile("cli"); delay_hpet(200); asm volatile("sti");
  printf("\n[Test Task 1] Completed, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  kill();
}


void test_task2()
{
  printf("\n[Test Task 2] Started, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  printf("\n%d", Get_Scheduler()->identity);
  delay1(1); asm volatile("cli"); delay_hpet(200); asm volatile("sti");
  printf("\n[Test Task 2] Completed, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  kill();
}


void test_task3()
{
  printf("\n[Test Task 3] Started, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  printf("\n%d", Get_Scheduler()->identity);
  delay1(1); asm volatile("cli"); delay_hpet(200); asm volatile("sti");
  printf("\n[Test Task 3] Completed, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  kill();
}

void test_task4()
{
  printf("\n[Test Task 4] Started, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  printf("\n%d", Get_Scheduler()->identity);
  delay1(1); asm volatile("cli"); delay_hpet(200); asm volatile("sti");
  printf("\n[Test Task 4] Completed, HPET Value : %d %d ...", *HPET_main_counter, *(HPET_main_counter+1));
  kill();
}