#include "task.c"
#include "process.c"

#include "ThreadTables/ThreadTable.c"
#include "Scheduler/Scheduler.c"
#include "IPCInterface/IPCInterface.c"
#include "ProcManager/ProcManager.c"
#include "Locks/Spinlocks.c"
#include "Syscalls/syscall_handling.c"

#include "test_tasks.c"

#include "FileSystem/vfs.h"

void idle_task()
{
  printf("\nHello World!");
  asm volatile("mov $0x4284, %eax; hlt");
  //printf("\nThis works!");
  //printf("\nArrived Here First %d", Get_Scheduler()->identity);
  //asm volatile("int $51");
 // kill();
  while(1);
    printf("%d ",*((uint32_t*)0xC2000000));
}

void init()
{
  printf("\nArrived Here First %d", Get_Scheduler()->identity);
  //asm volatile("int $51");
 // kill();
  while(1);
    printf("%d ",*((uint32_t*)0xC2000000));
}

void Spawner_Task()
{
  printf("\nArrived Here!!! %d", Get_Scheduler()->identity);
 // kill();
  while(1);
}

extern Aq_ListEntrys();

void init_multitasking()
{
  // Lets first setup the Scheduler and supporting structures for each core
  printf("\nInitializing Schedulers and creating Scheduler Structures...");
  Init_Schedulers();
  printf(" %g[Done]%g", 2, 15);
  ThreadTable_init();
  printf("\nInitializing Thread Tables...");
  printf(" %g[Done]%g", 2, 15);


  // Now we create some default processes like the kernel
  printf("\nCreating Kernel and Scheduler Processes...");
  Process_t* kernel = create_process("Kernel", 0, 0, NULL);
  kernel_proc = kernel;
  kernel->pgdir = system_dir;


  // IPC is a system to allow for sharing of commands between different cores
  printf("\nInitializing Inter Processor Control Sharing System...");
  IPC_init();
  printf(" %g[Done]%g", 2, 15);


  // SAS is a system to help the scheduler maintain its intigrity
  printf("\nInitializing Scheduler Assistance System...");
  SAS_proc = create_process("SAS_process", 0, 0, kernel);
  SAS_proc->pgdir = system_dir;
  SAS_init();
  printf(" %g[Done]%g", 2, 15);


  //printf("\nCreating few idle tasks...");
  Activate_task_direct(create_task("init", &init, 1, 0x202, kernel));
  Process_t* testp = create_process("TestProcess", 0, 0, NULL);
  Activate_task_direct(create_task("test", &idle_task, 1, 0x202, testp));
  printf(" %g[Done]%g", 2, 15);

  //switch_directory(testp->pgdir);
  //printf("\n It Works!");
  //asm volatile("mov $0x4284, %eax; hlt");


  // The next code would initialize the APIC Interrupts and Timers...
  printf("\nInitializing and Enabling APIC Interrupts and Timer driven Scheduling...");
  // *((uint32_t*)0xC2000000) = 0;
  apic_start_timer(APIC_LOCAL_BASE, 51);                      //The respective Timer initialization function of the timer of choice
  localapic_write(APIC_LOCAL_BASE, LAPIC_SVR, 0x100|0xff);    // Enables Interrupts on the APIC
  multitasking_ON = 1;
  printf(" %g[Done]%g", 2, 15);
  
  printf("\nWelcome to the Multitasking Realm...");
  // The Next line configures, enables interrupts and sets all APs to execute their scheduler code...
  *(uint32_t*)(0x3000 + 8 + AP_startup_Code_sz) = 0x3241;

  // The Next line enables interrupts on the base processor, and thus no core would follow this code anymore...
  asm volatile("sti");
  printf(" %g[Done]%g", 2, 15);
  while(1);
}

/*task_t* test = create_task("test_task", &Spawner_Task, 2, 0x202, kernel);
  Activate_task_direct(test);
  Activate_task_direct(create_task("test_task1", &test_task1, 1, 0x202, kernel));
  Activate_task_direct(create_task("test_task1", &test_task2, 1, 0x202, kernel));
  Activate_task_direct(create_task("test_task1", &test_task3, 1, 0x202, kernel));
  Activate_task_direct(create_task("test_task1", &test_task4, 1, 0x202, kernel));*/