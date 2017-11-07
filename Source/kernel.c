#include "Console/Console.c"
#include "library.c"
#include "interrupts/int_handlers.c"

void kernel_start()
{
    console_init();     // Initialize Console eg., init_console();
    
    printf("\nThis SHIT WORKS!!!!");
    
    init_descriptor_tables();	// Setup Descriptor tables for BSP IDT, GDT

    mem_init(); // Initialize Memory Management
    
    initAcpi(); // APIC, PIT, PIC initialize
    BasicCPU_Init();  // Initialize other Application Processors
    
    Setup_Paging();     // Initialize Paging
    Create_ScodeTables();// Keyboard Initialize
    init_tasking();
    asm volatile("sti");
    // Filesystem Initialize    -> NOT TO BE DONE
    // Input/Output System Initialize
    // Scheduler Initialize 
    // Process Management System Initialize -> thread
    // Shell Initialize -> thread
    // Initialize Multitasking
    // Inifinite Loop.
    while(1);
    return;
}