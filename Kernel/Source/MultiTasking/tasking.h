#ifndef TASKING_h
#define TASKING_h

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "Memory/mem.h"
#include "stdint.h"
#include "task.h"

//uint32_t QUEUE_START=33554432;//209715200
//uint32_t LAST_QUEUE=33636352;//209797120
#define TOTAL_QUEUES 20
const uint32_t TIME_MASK=0x000ff000;

//uint32_t* volatile top_queue=(uint32_t*)33636352;
//volatile uint32_t reached_bottom=0,bottom_task=1;
int volatile cli_already = 0;
volatile int multitasking_ON = 0;

//uint32_t *bottom_queue = (uint32_t *)33636352;
volatile uint32_t sas_bottom_task = 1;

//task_t* current_task,*old_task;

//extern uint32_t current_task;
//extern uint32_t old_task;
extern uint32_t old_process;
extern uint32_t new_process;
extern uint32_t current_esp;
extern uint32_t old_esp;
//extern uint32_t time_slice;

void Spawner_Task();
void test_process();

void MP_init_Sequence();
void tasking_initiator();
void init_multitasking();

extern void Scheduler_init_t();
extern void Scheduler_init_end_t();
extern void Scheduler_init();
void Spurious_task_func_t();
void Spurious_task_func_end_t();

void Terminator_vector();

#endif
