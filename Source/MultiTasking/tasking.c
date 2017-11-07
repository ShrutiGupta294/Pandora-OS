#include "tasking.h"
#include "library.h"

void Spawner_Task()
{
  //kill();
  printf("\nThis Shit is WORKING!");
  while(1);
}

void init_tasking()
{
    Init_Scheduler();

   // init_shell();

    //Here it goes, The entry to the multitasking world.

    for(uint32_t i = 0; i < total_CPU_Cores - 1; i++)
    {
        KitList[i+1].current_task = create_task("System_Spawner", &Spawner_Task, 0, 0x202);
        Activate_task_direct_SP((task_t*)(KitList[i+1].current_task), &KitList[i+1]);
        printf("\nCore %d", i);
        *(uint32_t*)(0x3000 + (i*0x2000) + AP_startup_Code_sz + 8) = 0x3240; //Notifies the cores that its time to start scheduling...
    }

    KitList[0].current_task = create_task("System_Spawner", &Spawner_Task, 0, 0x202);
    Activate_task_direct_SP((task_t*)(KitList[0].current_task), &KitList[0]);

    Activate_task_direct_SP((task_t*)(create_task("System_Spawner", &Spawner_Task, 0, 0x202)), &KitList[0]);

    clearIRQMask(1);
    apic_start_timer(APIC_LOCAL_BASE);       //The respective Timer initialization function of the timer of choice
    asm volatile("int $50");
    while(1);
}