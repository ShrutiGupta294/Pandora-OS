#ifndef SCHEDULER_H 
#define SCHEDULER_H 

uint32_t* MotherSpace;

typedef struct __attribute__((packed)) SchedulerKits
{
  uint32_t identity;
  uint32_t tasks;

  uint32_t* queue;

  uint32_t* scheduler;
  uint32_t* switcher;
  uint32_t* Scheduler_init;

  uint32_t current_task;

  uint32_t* stack;
  uint32_t* stack_top;
  uint32_t  tmp_threadStack;

  uintptr_t gdt;
  uintptr_t idt;
}SchedulerKits_t;

SchedulerKits_t* KitList;


void __attribute__((optimize("O0"))) Init_Scheduler();
SchedulerKits_t* __attribute__((optimize("O0"))) FindLightestScheduler();
SchedulerKits_t* __attribute__((optimize("O0"))) Get_Scheduler();
void Scheduler_t();
void Scheduler_end_t();

/********************************************************************************************/
//void Scheduler();

#endif 