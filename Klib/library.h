#ifndef LIBRARY_H 
#define LIBRARY_H

#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "common.h"
#include "sys.h"
#include "rand.h"

#include "acpi/acpi.h"
#include "apic/apic.h"
#include "apic/lapic.h"
#include "apic/ioapic.h"
#include "cpu/cpu.h"
#include "cpu/cpuid_c.h"
#include "Descriptors/descriptor_tables.h"
#include "MemManagement/mem.h"
#include "MemManagement/paging.h"
#include "Shell/Shell.h"
#include "Drivers/Timers/timer.h"
#include "Drivers/Timers/cmos.h"
#include "Drivers/Keyboard/Keyboard.h"


#include "MultiTasking/thread.h"
#include "MultiTasking/tasking.h"
#include "MultiTasking/Scheduler.h"


#endif 