#include "lapic.h"
#include "apic.h"
#include "ioapic.h"

/************************************ PIC ************************************/

void setIRQMask(int _IRQ)
{
	unsigned short port;
	unsigned char  mask;

	if(_IRQ < 8) {
		port = 0x21;
	} else {
		port = 0xA1;
		_IRQ -= 8;
	}
	mask = inb(port) | (1 << _IRQ);
	outb(port, mask);
}

void clearIRQMask(int _IRQ)
{
	unsigned short port;
	unsigned char  mask;

	if(_IRQ < 8) {
		port = 0x21;
	} else {
		port = 0xA1;
		_IRQ -= 8;
	}
	mask = inb(port) & ~(1 << _IRQ);
	outb(port, mask);
}

void enable_pic()
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void disable_pic()
{
    /* Set OCW1 (interrupt masks) */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
}

/************************************ APIC ************************************/

bool check_apic()
{
   uint32_t eax, ebx, ecx, edx;
   cpuid(1, eax, ebx, ecx, edx);
   return edx & CPUID_FLAG_APIC;
}

uint32_t* cpu_get_apic_base()
{
   uint32_t eax, edx;
   cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
  // cpuSetMSR((IA32_APIC_BASE_MSR)|(1<<11), &eax, &edx);

   return (uint32_t*)(eax & 0xfffff000);
}

void __attribute__((optimize("O0"))) BSP_init_LAPIC()
{

  enable_pic();
  disable_pic();
  APIC_LOCAL_BASE = (uint32_t)cpu_get_apic_base();

  AP_init_LAPIC();

  Lapic = (LAPIC_RegisterMAP_t*)APIC_LOCAL_BASE;
  uint32_t version_lapic = Lapic->lapicVER_Reg[0];
  version_lapic &= 0xff;
}

void __attribute__((optimize("O0"))) AP_init_LAPIC()
{
  localapic_write(APIC_LOCAL_BASE, LAPIC_ERROR, 0x1F); /// 0x1F: temporary vector (all other bits: 0)
  localapic_write(APIC_LOCAL_BASE, LAPIC_TPR, 0);

  localapic_write(APIC_LOCAL_BASE, LAPIC_DFR, 0xffffffff);
  localapic_write(APIC_LOCAL_BASE, LAPIC_LDR, 0x01000000);
  localapic_write(APIC_LOCAL_BASE, LAPIC_SVR, 0x100|0xff);
}

void MADTapic_parse()
{
  printf("\nInitializing IO APIC!!!");
  uint_fast32_t *ptr = (uint_fast32_t*)acpiGetRSDPtr();
  // check if address is correct  ( if acpi is available on this pc )
  if (ptr != NULL && acpiCheckHeader((uint32_t*)ptr, "RSDT") == 0)
  {
    //Get the APIC and HPET tables!!!
    struct RSDT* rsdt = (struct RSDT*)ptr;
    uint_fast32_t entries = rsdt->h.length;
    entries = (entries - sizeof(struct ACPI_SDTHeader))/4;
    ptr = (uint_fast32_t*)&rsdt->PointerToOtherSDT;

    for(uint32_t i=0; i<entries; i++)
    {
      if (acpiCheckHeader((uint32_t*)*ptr, "APIC") == 0)
      {
        printf("\nGot the MADT Structure");
        madt = (MADT_t*)*ptr;
        printf("\nLength: %d", madt->Length);
        madt_entry_t* madt_entry = (madt_entry_t*)&madt->rest_fields;
        lapic_entry_t* tmpLapic;
        while((uint32_t)madt_entry - (uint32_t)madt <= madt->Length)
        {
          switch(madt_entry->type)
          {
            case 0 :
                tmpLapic = (lapic_entry_t*)&madt_entry->rest_field;
                madt_entry = (madt_entry_t*)tmpLapic->rest_fields;
                if(!(tmpLapic->flags & (1<<0))) // Unusable LAPIC
                {
                  break;
                }
                ++total_CPU_Cores;
                printf("\n\tLocal APIC Found");
                printf("\t\t\t%gLAPIC USABILITY: %x%g", 10, tmpLapic->flags & (1<<0),0);
                LAPIC_entry = tmpLapic;
                //return;
              break;
            case 1 :
                printf("\n\tIO APIC Found");
                IOAPIC_entry = (ioapic_entry_t*)&madt_entry->rest_field;
                uint32_t maxintr = (ioapic_read(IOAPIC_REG_VER) >> 16) & 0xFF;
                printf(" id: %x, address: %x, GSIB: %x, Max Ints: %d",IOAPIC_entry->id, IOAPIC_entry->address, IOAPIC_entry->gsib, maxintr);
                APIC_IO_BASE = IOAPIC_entry->address;
                madt_entry = (madt_entry_t*)IOAPIC_entry->rest_fields;

                uint32_t id = ioapic_read(IOAPIC_REG_ID) >> 24;
                if(id != IOAPIC_entry->id)
                  printf("\nioapicinit: id isn't equal to ioapicid; not a MP-> %d %d", id, IOAPIC_entry->id);
                //while(1);
               // if(id == 0)
               //   Init_ioapic(IOAPIC_entry);//*/
              break;
            case 2 :
                printf("\n\tInterrupt Source Override Found");
                ISD_entry = (isd_entry_t*)&madt_entry->rest_field;
                madt_entry = (madt_entry_t*)ISD_entry->rest_fields;
              break;
            default:
              //  printf(" %x ", madt_entry->type);
              //  printf("\n\tUnknown entry type");
                i=1;
                return;
              break;
          }
        }
        return;
      }
      ptr++;
    }
  }
}
