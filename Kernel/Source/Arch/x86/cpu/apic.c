#include "apic.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "Arch/x86/cpu/Lapic/lapic.h"
#include "Arch/x86/cpu/IOapic/ioapic.h"
#include "interrupts/interrupts.h"

bool check_apic()
{
  uint32_t eax, ebx, ecx, edx;
  cpuid(1, eax, ebx, ecx, edx);
  return edx & CPUID_FLAG_APIC;
}

uint32_t *cpu_get_apic_base()
{
  uint32_t eax, edx;
  cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
  // cpuSetMSR((IA32_APIC_BASE_MSR)|(1<<11), &eax, &edx);

  return (uint32_t *)(eax & 0xfffff000);
}

void __attribute__((optimize("O0"))) BSP_init_LAPIC(uint32_t base)
{
  //  clearIRQMask(0);
  //  clearIRQMask(1);
  //localapic_write_with_mask(LAPIC_SVR, (1<<8), (1<<8));

  //  printf("\n\nTesting APIC! Local APIC revision: %x Max LVT entry: %x\n",localapic_read(base, LAPIC_VER)&0xff, ((localapic_read(base, LAPIC_VER)>>16) & 0xff)+1);
  /*
  // Aqeous LAPIC Init Series, commented out in favor of that borrowed from xv6
  localapic_write(base, LAPIC_ERROR, 0x1F); /// 0x1F: temporary vector (all other bits: 0)
  localapic_write(base, LAPIC_TPR, 0);

  localapic_write(base, LAPIC_DFR, 0xffffffff);
  localapic_write(base, LAPIC_LDR, 0x01000000);
  localapic_write(base, LAPIC_SVR, 0x100 | 0xff);
*/
  //  uint32_t eax, edx;
  //  cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
  //  cpuSetMSR((IA32_APIC_BASE_MSR)|(1<<11), &eax, &edx);

  init_LAPIC();
  Lapic = (LAPIC_RegisterMAP_t *)base;
  uint32_t version_lapic = Lapic->lapicVER_Reg[0];
  version_lapic &= 0xff;
}

void __attribute__((optimize("O0"))) AP_init_LAPIC()
{
  localapic_write(APIC_LOCAL_BASE, LAPIC_ERROR, 0x1F); /// 0x1F: temporary vector (all other bits: 0)
  localapic_write(APIC_LOCAL_BASE, LAPIC_TPR, 0);

  localapic_write(APIC_LOCAL_BASE, LAPIC_DFR, 0xffffffff);
  localapic_write(APIC_LOCAL_BASE, LAPIC_LDR, 0x01000000);
  localapic_write(APIC_LOCAL_BASE, LAPIC_SVR, 0x100 | 0xff);
}

void init_LAPIC() // xv6 Port
{
  // Enable local APIC; set spurious interrupt vector.
  localapic_write(APIC_LOCAL_BASE, LAPIC_SVR, (T_IRQ0 + IRQ_SPURIOUS));

  // The timer repeatedly counts down at bus frequency
  // from lapic[TICR] and then issues an interrupt.
  // If xv6 cared more about precise timekeeping,
  // TICR would be calibrated using an external time source.
  localapic_write(APIC_LOCAL_BASE, LAPIC_TDCR, LAPIC_X1);
  localapic_write(APIC_LOCAL_BASE, LAPIC_TIMER, LAPIC_PERIODIC | 51);
  localapic_write(APIC_LOCAL_BASE, LAPIC_TICR, 10000000);

  // Disable logical interrupt lines.
  localapic_write(APIC_LOCAL_BASE, LAPIC_LINT0, LAPIC_MASKED);
  localapic_write(APIC_LOCAL_BASE, LAPIC_LINT1, LAPIC_MASKED);

  // Disable performance counter overflow interrupts
  // on machines that provide that interrupt entry.
  if (((((uint32_t *)APIC_LOCAL_BASE)[LAPIC_VER / 4] >> 16) & 0xFF) >= 4)
    localapic_write(APIC_LOCAL_BASE, LAPIC_PERF, LAPIC_MASKED);

  // Map error interrupt to IRQ_ERROR.
  localapic_write(APIC_LOCAL_BASE, LAPIC_ERROR, T_IRQ0 + IRQ_ERROR);

  // Clear error status register (requires back-to-back writes).
  localapic_write(APIC_LOCAL_BASE, LAPIC_ESR, 0);
  localapic_write(APIC_LOCAL_BASE, LAPIC_ESR, 0);

  // Ack any outstanding interrupts.
  localapic_write(APIC_LOCAL_BASE, LAPIC_EOI, 0);
  /*
  // Send an Init Level De-Assert to synchronise arbitration ID's.
  localapic_write(APIC_LOCAL_BASE, LAPIC_ICRHI, 0);
  localapic_write(APIC_LOCAL_BASE, LAPIC_ICRLO, LAPIC_BCAST | LAPIC_INIT | LAPIC_LEVEL);
  while (((uint32_t *)APIC_LOCAL_BASE)[LAPIC_ICRLO] & LAPIC_DELIVS)
    ;
*/
  // Enable interrupts on the APIC (but not on the processor).
  localapic_write(APIC_LOCAL_BASE, LAPIC_TPR, 0);
  /*
  localapic_write(APIC_LOCAL_BASE, LAPIC_ERROR, 0x1F); /// 0x1F: temporary vector (all other bits: 0)
  localapic_write(APIC_LOCAL_BASE, LAPIC_TPR, 0);

  localapic_write(APIC_LOCAL_BASE, LAPIC_DFR, 0xffffffff);
  localapic_write(APIC_LOCAL_BASE, LAPIC_LDR, 0x01000000);
  localapic_write(APIC_LOCAL_BASE, LAPIC_SVR, 0x100|0xff);//*/
}

void apic_start_timer(uint32_t base, uint32_t intnum)
{ /*
  localapic_write(base, LAPIC_DFR, 0xffffffff);
  localapic_write(base, LAPIC_LDR, 0x01000000);
  localapic_write(base, LAPIC_SVR, 0x100|0xff);
  */
  // Tell APIC timer to use divider 4
  localapic_write(base, LAPIC_TDCR, 0x3);

  // Prepare the PIT to sleep for 10ms (10000µs)
  //pit_prepare_sleep(10000);

  // Set APIC init counter to -1
  localapic_write(base, LAPIC_TICR, 0xFFFFFFFF);

  // Perform PIT-supported sleep
  //pit_perform_sleep();
  // Stop the APIC timer
  localapic_write(base, LAPIC_TIMER, 0x10000);

  // Now we know how often the APIC timer has ticked in 10ms
  //  uint32_t ticksIn10ms = 0xFFFFFFFF - localapic_read(LAPIC_TCCR);
  
  localapic_write(APIC_LOCAL_BASE, LAPIC_TDCR, 0x3);
  localapic_write(APIC_LOCAL_BASE, LAPIC_TIMER, LAPIC_FIXED | intnum);
  localapic_write(APIC_LOCAL_BASE, LAPIC_TICR, 10000000);//*/
}

void MADTapic_parse()
{
  printf("\nInitializing IO APIC!!!");
  uint_fast32_t *ptr = (uint_fast32_t *)acpiGetRSDPtr();
  // check if address is correct  ( if acpi is available on this pc )
  if (ptr != NULL && acpiCheckHeader((uint32_t *)ptr, "RSDT") == 0)
  {
    //Get the APIC and HPET tables!!!
    struct RSDT *rsdt = (struct RSDT *)ptr;
    uint_fast32_t entries = rsdt->h.length;
    entries = (entries - sizeof(struct ACPI_SDTHeader)) / 4;
    ptr = (uint_fast32_t *)&rsdt->PointerToOtherSDT;

    for (uint32_t i = 0; i < entries; i++)
    {
      if (acpiCheckHeader((uint32_t *)*ptr, "APIC") == 0)
      {
        printf("\nGot the MADT Structure");
        madt = (MADT_t *)*ptr;
        printf("\nLength: %d", madt->Length);
        madt_entry_t *madt_entry = (madt_entry_t *)&madt->rest_fields;
        lapic_entry_t *tmpLapic;
        while ((uint32_t)madt_entry - (uint32_t)madt <= madt->Length)
        {
          switch (madt_entry->type)
          {
          case 0:
            tmpLapic = (lapic_entry_t *)&madt_entry->rest_field;
            madt_entry = (madt_entry_t *)tmpLapic->rest_fields;
            if (!(tmpLapic->flags & (1 << 0))) // Unusable LAPIC
            {
              break;
            }
            ++total_CPU_Cores;
            printf("\n\tLocal APIC Found");
            printf("\t\t\t%gLAPIC USABILITY: %x%g", 10, tmpLapic->flags & (1 << 0), 0);
            LAPIC_entry = tmpLapic;
            //return;
            break;
          case 1:
            printf("\n\tIO APIC Found");
            IOAPIC_entry = (ioapic_entry_t *)&madt_entry->rest_field;
            uint32_t maxintr = (ioapic_read(IOAPIC_REG_VER) >> 16) & 0xFF;
            printf(" id: %x, address: %x, GSIB: %x, Max Ints: %d", IOAPIC_entry->id, IOAPIC_entry->address, IOAPIC_entry->gsib, maxintr);
            APIC_IO_BASE = IOAPIC_entry->address;
            madt_entry = (madt_entry_t *)IOAPIC_entry->rest_fields;

            uint32_t id = ioapic_read(IOAPIC_REG_ID) >> 24;
            if (id != IOAPIC_entry->id)
              printf("\nioapicinit: id isn't equal to ioapicid; not a MP-> %d %d", id, IOAPIC_entry->id);
            //while(1);
            if (id == 0)
              Init_ioapic(IOAPIC_entry); //*/
            break;
          case 2:
            printf("\n\tInterrupt Source Override Found");
            ISD_entry = (isd_entry_t *)&madt_entry->rest_field;
            madt_entry = (madt_entry_t *)ISD_entry->rest_fields;
            break;
          default:
            //  printf(" %x ", madt_entry->type);
            //  printf("\n\tUnknown entry type");
            i = 1;
            return;
            break;
          }
        }
        return;
      }
      ptr++;
    }
  }
  while (1)
    ;
}
