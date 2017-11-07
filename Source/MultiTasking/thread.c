#include "thread.h"
#include "library.h"

task_t* create_task(char* name, func_t func, uint32_t priority, uint32_t flags)  /// Create a New Task for a given Process
{
	task_t* New_task = (task_t*)pmem_4k(2);
	uint32_t* stack = (uint32_t*)New_task;

    memset_fast((void*)stack,0,4096);
    
	strcpy(New_task->name, name);
	New_task->task_id = task_counter;

	//New_task->main_pgdir = process->pgdir;
	New_task->mem_used = 0;

	stack += 1024;
	//stack += 512;

	uint32_t base=(uint32_t)stack;

	*--stack = flags; //EFLAGS
	*--stack = 0x8; //cs
	*--stack = (uint32_t)func; //eip

//	*--stack = ds;		// ds, es, fs, gs

	*--stack = 0x4284; // eax
	*--stack = 0; // ecx
	*--stack = 0; // edx;
	*--stack = 0; //ebx
	*--stack = (uint32_t)(base-16); //esp
	*--stack = (uint32_t)(base-16); //ebp
	*--stack = 0; //esi
	*--stack = 0; //edi


	New_task->esp = (uint32_t)stack;
	New_task->tokens = priority;
	New_task->func = (uint32_t)func;

	//TODO: Complete this function, check everything thrice
	printf("{%x}", New_task);
	return New_task;
}

task_t* __attribute__((optimize("O0"))) task_GetSelf()
{
	return (task_t*)(Get_Scheduler()->current_task);
}

void __attribute__((optimize("O0"))) Activate_task_direct(task_t* task) /// Put a given Task_entry into an appropriate queue for it to be executed.
{
	SchedulerKits_t* kit = FindLightestScheduler();
	Activate_task_direct_SP(task, kit);
}

void __attribute__((optimize("O0"))) Activate_task_direct_SP(task_t* task, SchedulerKits_t* kit) /// Put a given Task_entry into an appropriate queue for it to be executed.
{
	
	task->Scheduler = (SchedulerKits_t*)kit;
	kit->queue[(uint32_t)(kit->queue[1])] = task;
	task->active = &kit->queue[(uint32_t)(kit->queue[1])];
	++(kit->queue[1]);
	++kit->tasks;
}

void __attribute__((optimize("O0"))) Task_Remover(uint32_t* place_holder, SchedulerKits_t* kit)		// Remove the task from scheduler queues
{
	*place_holder = 0;
}

void __attribute__((optimize("O0"))) kill()	// Know on which core the kill function has been called. Then get the scheduler of the core and get the current_task
{
	//LOCK(TASK_LOCK_KILL);
	asm volatile("cli");
	SchedulerKits_t* kit = Get_Scheduler();//Get the Scheduler of the core on which this instruction is executed.

	uint32_t *place_holder = (uint32_t *)((task_t*)kit->current_task)->active;
	Task_Remover(place_holder, kit);

	kit->current_task = (uint32_t)Idle_task;
	--kit->tasks;
	//UNLOCK(TASK_LOCK_KILL);
//	while(1);
}

void __attribute__((optimize("O0"))) Task_sleep(task_t* task)
{
	SchedulerKits_t* kit = task->Scheduler;
	Task_Remover((uint32_t*)(task->active), kit);
}

void __attribute__((optimize("O0"))) Task_wakeup(task_t* task)
{
	*((uint32_t*)(task->active)) = (uint32_t)task;
}

