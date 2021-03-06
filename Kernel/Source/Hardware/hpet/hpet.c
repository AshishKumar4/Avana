#include "hpet.h"
#include "stdio.h"
#include "string.h"
#include "Arch/x86/cpu/apic.h"
#include "Arch/x86.h"

inline unsigned int hpet_readl(unsigned int a)
{
	return readl(hpet_virt_address + a);
}

static inline void hpet_writel(unsigned int d, unsigned int a)
{
	writel(d, hpet_virt_address + a);
}


void init_hpet()
{
  printf("\nInitializing HPET (High Precision Event Timer)...");
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
      if (acpiCheckHeader((uint32_t*)*ptr, "HPET") == 0)
      {
        //TODO: INITIALIZE HPET
        hpet_sdt = (HPET_descriptor_table_t*)*ptr;
        printf("\n%g[Done]%g HPET Initialized\n", 10, 15);
        hpet_base = (uint32_t*)hpet_sdt->Base_address[1];
        hpet_virt_address = (uint32_t)hpet_base;
        hpet = (HPET_Table_t*)hpet_base;
        hpet->Main_Counter_Reg = 0;

			//	ioapic_set_irq(2, IOAPIC_entry->id, )	 48 = 110000

        hpet->GCReg[0] = 1;
				//hpet->Timer0CCReg[0] = (2 << 9) | (1 << 2) | (1 << 3) | (1 << 6);//10000;
      //  hpet->Timer0CCReg[0] |= 0b00001101001110; //0b000011101001100
			//	hpet->Timer0CVReg[0] = 0x10000;
				//hpet->Timer0CCReg[0] |=
				//printf("\nGeneral Capabilities reg: %d GISReg: %d GCReg: %d \nTimer0 Capabilities: %d %d\n", hpet->GCIDReg[0], hpet->GISReg[0], hpet->GCReg[0], hpet->Timer0CCReg[1], hpet->Timer0CCReg[0]);
			//	printf("-%x \nMinimum Tick period: %x ",hpet->Main_Counter_Reg,hpet->GCIDReg[1]);
        //hpet->GCIDReg[1] = 10000;

				/*for(int i = 0; i < 10000; i++)
				{
					printf("-%x",hpet->Main_Counter_Reg);
				}*/
				HPET_main_counter = (uint32_t*)&hpet->Main_Counter_Reg;
				break;
      }
      ptr++;
    }
  }
}

void __attribute__((optimize("O0"))) delay_hpet(uint32_t ms)
{
	if(!hpet_base)
	{
		init_hpet();
		return;
	}
	uint32_t tperiod = ms*100000;
	uint32_t tcounter = tperiod;
	tcounter += *HPET_main_counter;
	while(tcounter >= *HPET_main_counter);
}
