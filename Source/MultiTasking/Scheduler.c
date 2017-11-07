#include "library.h"
#include "scheduler.h"

extern void switcher_end_t();
extern void switcher_ksp_t();

void __attribute__((optimize("O0"))) Init_Scheduler()
{
  //Create copies of Scheduler function and switcher functions based on number of CPU cores.
  //UNLOCK(CORE_INFO);
  uint32_t sz_Scheduler = Scheduler_end_t - Scheduler_t;
  uint32_t sz_switcher = switcher_end_t - switcher_ksp_t;
  uint32_t* nspace;
  uint8_t *nptr1;

  MotherSpace = (uint32_t*)phy_alloc4K();
  memset(MotherSpace, 0, 4096);
  SchedulerKits_t* kits = (SchedulerKits_t*)MotherSpace;

  for(uint32_t i = 0; i < total_CPU_Cores; i++)
  {
    printf("\nCore #%x Initialized",i+1);
    nspace = (uint32_t*)phy_alloc4K();
    memset((void*)nspace, 0, 4096);
    kits->identity = i;
    kits->tasks = 0;

    nptr1 = (uint8_t*)nspace;
    memcpy((void*)nptr1, switcher_ksp_t, sz_switcher);
    kits->switcher = (uint32_t*)nptr1;

    nptr1 += sz_switcher;
    nptr1 = ROUNDUP(nptr1,4) + 32;
    memcpy((void*)nptr1, Scheduler_t, sz_Scheduler);
    kits->scheduler = (uint32_t*)nptr1;


    nptr1 += sz_Scheduler;
    nptr1 = ROUNDUP(nptr1,4) + 32;

    uint32_t* temporary_stack = (uint32_t*)phy_alloc4K();//kmalloc(4096*4);
    kits->stack_top = temporary_stack;

    temporary_stack += 512;

    kits->stack = temporary_stack;

    ByteSequence_Replace(0x4284ACD2, 4, (uint32_t)&system_dir, 4, kits->switcher, kits->switcher + sz_switcher); //system_dir
    ByteSequence_Replace(0x4284ACD3, 4, (uint32_t)&kits->current_task, 4, kits->switcher, kits->switcher + sz_switcher); //current_task
    ByteSequence_Replace(0x4284ABD4, 4, (uint32_t)&kits->current_task, 4, kits->scheduler, kits->scheduler + sz_Scheduler); //current_task
    ByteSequence_Replace(0x4284ACD5, 4, (uint32_t)&kits->scheduler, 4, kits->switcher, kits->switcher + sz_switcher); //Scheduler
    ByteSequence_Replace(0x4284CD00, 4, (uint32_t)temporary_stack, 4, kits->switcher, kits->switcher + sz_switcher); //Temporary Stack
    ByteSequence_Replace(0x4284ABC1, 4, (uint32_t)&kits->queue, 4, kits->scheduler, kits->scheduler + sz_Scheduler); //LAST_QUEUE

    uint32_t pmode_code_addr = (0x1000 + (i*0x2000)) + AP_startup_Code_sz + 16;
    if(i)
      ByteSequence_Replace(0x42842222, 4, (uint32_t)&Spawner_Task, 4, (uint32_t*)pmode_code_addr, (uint32_t*)(pmode_code_addr + pmode_code_size));

    kits->queue = (uint32_t*)pmem_4k(40);//kmalloc(4096*40);//mtalloc(20);//33554432;
    memset((void*)kits->queue, 0, 4096*40);
    ((uint32_t*)(kits->queue))[0] = 2;
    ((uint32_t*)(kits->queue))[1] = 2;

    // Setup a Cleaner Better Descriptors for all Processors
    kits->gdt = pmode_GDT_init(i);
    //pmode_TSS_ltr(i, 0x2B);
    kits->idt = pmode_IDT_initP(i);

    uint32_t* idt_ptr = (uint32_t*)(kits->idt);
    idt_ptr += 2;
    idtSetEntry(50, (uint32_t)kits->switcher, 0x08, makeFlagByte(1, USER_MODE), (uint64_t*)idt_ptr);
    idtSetEntry(51, (uint32_t)kits->switcher, 0x08, makeFlagByte(1, KERNEL_MODE), (uint64_t*)idt_ptr);
    idtSetEntry(13, (uint32_t)&generalProtectionFault_handler, 0x08, makeFlagByte(1, USER_MODE), (uint64_t*)idt_ptr);

    pmode_IDT_lidt(i, kits->idt);

    ++kits;
  }
  //while(1);
  KitList = (SchedulerKits_t*)MotherSpace;
}

SchedulerKits_t* __attribute__((optimize("O0"))) FindLightestScheduler()
{
  SchedulerKits_t* tmp = (SchedulerKits_t*)MotherSpace;
  SchedulerKits_t* tmp2 = tmp;
//  ++tmp;
  for(uint32_t i = 0; i < total_CPU_Cores; i++)
  {
    if(tmp2->tasks > tmp->tasks) tmp2 = tmp;
    ++tmp;
  }
  return tmp2;
}

SchedulerKits_t* __attribute__((optimize("O0"))) Get_Scheduler()
{
  uint32_t i;
  i = *(uint32_t*)(0xfee00020);
  i = i>>24;
  return &KitList[i];
}

void __attribute__((optimize("O0"))) Scheduler_t()    // Scheduler Template
{
    uint32_t* ThreadQ = *((uint32_t*)0x4284ABC1);
  // asm volatile("mov %0, %%eax; mov %1, %%ebx; hlt" :: "r"(ThreadQ), "r"(ThreadQ[0]));
    uint32_t* currentQ_element = &ThreadQ[ThreadQ[0]];
    back:
    if(&ThreadQ[(uint32_t)ThreadQ[0]] <= currentQ_element)  // Next Task is the task from start of queue because we reached the end.
    {
        ThreadQ[0] = 2;
        currentQ_element = &ThreadQ[2];
    }
    else    // Next Task is the task next in the queue
    {
        ++ThreadQ[0];
        ++currentQ_element;// = &ThreadQ[ThreadQ[0]];
    }
    if(!(*currentQ_element))
    {
      //asm volatile("mov $0x4284, %eax; hlt");
      goto back;
    }
    *((uint32_t*)(0x4284ABD4)) = *currentQ_element;
    //asm volatile("mov %0, %%eax; mov %1, %%ebx; hlt" :: "r"(currentQ_element), "r"(currentQ_element[0]));
}

void Scheduler_end_t()
{
  return;
}

/********************************************************************************************/
/*void Scheduler()    // Round Robin Simple
{
    back:
    if(&ThreadQ[(uint32_t)ThreadQ[0]] >= currentQ_element)  // Next Task is the task from start of queue because we reached the end.
    {
        ThreadQ[0] = 1;
        currentQ_element = &ThreadQ[1];
    }
    else    // Next Task is the task next in the queue
    {
        ++ThreadQ[0];
        CurrentQ_element = &ThreadQ[ThreadQ[0]];
    }
    if(!(*CurrentQ_element)) goto back;        // In case tasks are not present
}*/