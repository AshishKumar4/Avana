#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "Processing/Scheduler/Scheduler.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

// Mutual exclusion lock.
struct spinlock
{
  uint32_t locked; // Is the lock held?

  // For debugging:
  char *name;           // Name of lock.
  SchedulerKits_t *cpu; // The cpu holding the lock.
  uint32_t pcs[10];         // The call stack (an array of program counters)
                        // that locked the lock.
};

#endif
