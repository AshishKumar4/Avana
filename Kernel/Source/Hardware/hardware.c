#include "hardware.h"

#include "Hardware/vga/vga.c"
#include "Hardware/acpi/acpi.c"
#include "Hardware/hpet/hpet.c"
#include "Hardware/pci/pci.c"
#include "Hardware/ahci/ahci.c"
//#include "Hardware/keyboard/keyboard.c"

int init_basic_hardware()
{
    init_acpi();
	if(!acpiEnable())
		printf("\n%g[Done]%g ACPI Initialized \n",10, 15);
	else printf("\n%g[Error]%g ACPI CANT BE INITIALIZED \n", 4, 15);
    init_hpet();
	printf("\n\nEnumerating all devices on PCI BUS:\n");
	checkAllBuses();
	printf("\t%g[Done]%g\n",10, 15);
	
	printf("\nEnabling Hard Disk\n");
	checkAHCI();
	printf("\t%g[Done]%g\n",10, 15);
	return 0;
}

