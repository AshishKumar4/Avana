#pragma once

#include "Arch/x86/sys.c"
#include "Arch/x86/cpu/cpu.c"
#include "Arch/x86/DescriptorTables/desc.c"

#include "interrupts/interrupts.c"

#include "Hardware/hardware.c"

#include "Memory/mem.c"

#include "FileSystem/AqFS/Aqfs.c"
#include "FileSystem/ext2/ext2_fs.c"
#include "FileSystem/vfs.c"

#include "Processing/tasking.c"

#include "Console/Console.c"

#include "Timer/timer.c"
#include "Timer/cmos.c"

#include "IO_Handling/std_iohandling.c"