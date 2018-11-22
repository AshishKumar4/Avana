#include "cpu.h"
#include "pic.c"
#include "Lapic/lapic.c"
#include "IOapic/ioapic.c"
#include "apic.c"

#include "Arch/x86.h"
#include "Hardware/hpet/hpet.h"

#include "../DescriptorTables/desc.h"

bool cpuHasMSR()
{
    uint32_t a, b, c, d; // eax, edx
    cpuid(1, a, b, c, d);
    return d & CPUID_FLAG_MSR;
}

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    asm volatile("rdmsr"
                 : "=a"(*lo), "=d"(*hi)
                 : "c"(msr));
}

void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
    asm volatile("wrmsr"
                 :
                 : "a"(lo), "d"(hi), "c"(msr));
}

static inline uint64_t rdtsc()
{
    uint64_t ret;
    asm volatile("rdtsc"
                 : "=A"(ret));
    return ret;
}

static inline void invlpg(void *m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile("invlpg (%0)"
                 :
                 : "b"(m)
                 : "memory");
}

int init_smp();
int ap_kickstart();

int init_cpu_x86()
{
    // Initialize PIC and then Disable it!
    printf("\nInitializing PIC!");
    enable_pic();
    printf(" %g[Done]%g", 2, 15);

    printf("\nDisabling PIC to enable APIC!");
    disable_pic();
    printf(" %g[Done]%g", 2, 15);

    printf("\nEnabling BSP LAPIC...");
    APIC_LOCAL_BASE = (uint32_t)cpu_get_apic_base();
    printf("\nLAPIC Address : %x", APIC_LOCAL_BASE);
    BSP_init_LAPIC(APIC_LOCAL_BASE); // BSP LAPIC Initialize
    BSP_id = Lapic->lapicID_Reg[0] && 0xff;
    printf(" %g[Done]%g", 2, 15);

    printf("\nParsing MADT Tables...");
    MADTapic_parse();
    printf(" %g[Done]%g", 2, 15);

    // Time to initialize all the CPU Cores...
    printf("\nTotal CPU Cores Found: %d", total_CPU_Cores);
    if (total_CPU_Cores > 1)
    {
        init_smp();
        for(uint32_t i = 1; i < total_CPU_Cores; i++)
        {
            ap_kickstart(i);
            //for(uint32_t i = 0; i < total_CPU_Cores - 1; i++)
            //*(uint32_t*)(0x3000 + AP_startup_Code_sz + 8) = 0x4284;
        }
    }
    return 0;
}

uint32_t vector_addr = 0x3000; // Default starting point for all APs
uint32_t *test_counter = (uint32_t *)0x00001000;
uint32_t *semaphore = (uint32_t *)0x00000504;

int init_smp()
{
    // Boot the AP's
    *(char *)(0xf) = 0xa;
    AP_startup_Code_sz = AP_startup_Code_end - AP_startup_Code;
    AP_startup_Code_sz = ((AP_startup_Code_sz / 4) + 1) * 4;

    uint32_t pmode_code_addr;
    pmode_code_size = pmode_AP_code_end - pmode_AP_code;
    pmode_code_size = ((pmode_code_size / 4) + 1) * 4;

    //  uint32_t APIC_error_vector = 0x1F000;

    *test_counter = 0;

    *semaphore = 0;

    CPU_BOOT_MODE = 0;

    memset((void *)vector_addr, 0, 4096);
    memcpy((void *)vector_addr, AP_startup_Code, AP_startup_Code_sz); // Copy the code of switching to protected mode into the bottom of stacks for each AP
    
    ByteSequence_Replace(0x4284, 2, (0x5000), 2, (uint32_t *)vector_addr, (uint32_t *)(vector_addr + AP_startup_Code_sz));

    uint32_t *gdt_new = (uint32_t *)(vector_addr + AP_startup_Code_sz + pmode_code_size + 16);
    SimpleGDT_Setup((uint32_t *)(vector_addr + AP_startup_Code_sz + pmode_code_size + 8 + 16), gdt_new);

    ByteSequence_Replace(0x3240, 2, (uint32_t)gdt_new, 2, (uint32_t *)vector_addr, (uint32_t *)(vector_addr + AP_startup_Code_sz));

    uint32_t *idt_new = (uint32_t *)(vector_addr + AP_startup_Code_sz + pmode_code_size + 16 + 48);
    SimpleIDT_Setup((uint32_t *)(vector_addr + AP_startup_Code_sz + pmode_code_size + 8 + 16 + 48), idt_new);

    ByteSequence_Replace(0x3250, 2, (uint32_t)idt_new, 2, (uint32_t *)vector_addr, (uint32_t *)(vector_addr + AP_startup_Code_sz));

    pmode_code_addr = vector_addr + AP_startup_Code_sz + 16;
    memcpy((void *)pmode_code_addr, pmode_AP_code, pmode_code_size); // Copy the code of switching to protected mode into the bottom of stacks for each AP
    ByteSequence_Replace(0x3260, 2, (uint32_t)pmode_code_addr, 2, (uint32_t *)vector_addr, (uint32_t *)(vector_addr + AP_startup_Code_sz));

    *(uint32_t *)(vector_addr + AP_startup_Code_sz + 8) = 0;
    *(uint32_t *)(vector_addr + AP_startup_Code_sz + 4) = 0;
    ByteSequence_Replace(0x5599, 2, vector_addr + AP_startup_Code_sz + 8, 2, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));
    ByteSequence_Replace(0x5566, 2, vector_addr + AP_startup_Code_sz + 4, 2, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));
    ByteSequence_Replace(0x4959, 2, pmode_code_addr, 2, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));

    ByteSequence_Replace(0x32409798, 4, (uint32_t)&ap_gdts, 4, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));
    ByteSequence_Replace(0x32409799, 4, (uint32_t)&ap_idts, 4, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));
    uintptr_t pg_dir;
    asm volatile("mov %%cr3, %%eax; mov %%eax, %0":"=r"(pg_dir)); 
    ByteSequence_Replace(0x12344321, 4, pg_dir, 4, (uint32_t *)pmode_code_addr, (uint32_t *)(pmode_code_addr + pmode_code_size));

    return 0;
}

int ap_kickstart(int apid)
{
    uint32_t vector = ((vector_addr) / 4096);
    uint32_t tmp_counter = *test_counter;
    localapic_write(APIC_LOCAL_BASE, LAPIC_ICRHI, (apid << 24)); // INIT IPI
    localapic_write(APIC_LOCAL_BASE, LAPIC_ICRLO, 0x00004500);   // INIT IPI

    //  Lapic->ICR[0][0] = 0x000C4500;
    delay_hpet(10);
    localapic_write(APIC_LOCAL_BASE, LAPIC_ICRHI, (apid << 24));        // INIT IPI
    localapic_write(APIC_LOCAL_BASE, LAPIC_ICRLO, 0x00004600 | vector); // SIPI IP
                                                                       //  Lapic->ICR[0][0] = 0x000C4600 + vector;
    delay_hpet(200);
    if ((*test_counter) == tmp_counter)
    {
        localapic_write(APIC_LOCAL_BASE, LAPIC_ICRHI, (apid << 24)); // INIT IPI
        printf("\nReatempting INIT SIPI");
        localapic_write(APIC_LOCAL_BASE, LAPIC_ICRLO, 0x00004600 | vector); // 2nd SIPI IP
    }
    delay_hpet(10);
    
    if ((*test_counter) != tmp_counter)
        printf("\n\t\t%gThe AP #%x has been Booted Successfully %x %g", 10, apid, *test_counter, 15);
    else 
    {
        printf("\n\t\t%g[ERROR] The AP #%x could not be booted%g", 4, apid, 15);
        return 1;
    }
    return 0;
}