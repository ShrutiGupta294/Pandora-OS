#ifndef THREADS_H
#define THREADS_H 

#include "library.h"
#include "Scheduler.h"

// This structure defines a 'task' - a process.
typedef struct __attribute__((packed)) _task //DO NOT CHANGE ANYTHING UNLESS YOU HAVE A REASON; Make changes in tasking.asm too then.
{
    uint32_t esp;                   //0
    uint32_t tokens;            //20
    uint32_t active;            //24
    uint32_t RtFlags;
    uint32_t mem_used;
    uint32_t task_id;
    uint32_t func;
    SchedulerKits_t* Scheduler;
    char name[50];
}task_t;

uint32_t task_counter = 0;

task_t *Kernel_task;
task_t *Idle_task, *Shell_task, *Shell_Istream_task, *Shell_Ostream_task;

task_t* create_task(char* name, func_t func, uint32_t priority, uint32_t flags);
task_t* __attribute__((optimize("O0"))) task_GetSelf();
void __attribute__((optimize("O0"))) Activate_task_direct(task_t* task);
void __attribute__((optimize("O0"))) Activate_task_direct_SP(task_t* task, SchedulerKits_t* kit);
void __attribute__((optimize("O0"))) Task_Remover(uint32_t* place_holder, SchedulerKits_t* kit);
void __attribute__((optimize("O0"))) kill();
void __attribute__((optimize("O0"))) Task_sleep(task_t* task);
void __attribute__((optimize("O0"))) Task_wakeup(task_t* task);

#endif 