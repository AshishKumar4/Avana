// Mutual exclusion spin locks.
#include "Spinlocks.h"
#include "Processing/Scheduler/Scheduler.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"

void initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = NULL;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock.
  if (holding(lk))
    panic("acquire");

  // The xchg is atomic.
  while (xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  lk->cpu = Get_Scheduler();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void release(struct spinlock *lk)
{
  if (!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  asm volatile("movl $0, %0"
               : "+m"(lk->locked)
               :);

  popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void getcallerpcs(void *v, uint32_t pcs[])
{
  uint32_t *ebp;
  int i;

  ebp = (uint32_t *)v - 2;
  for (i = 0; i < 10; i++)
  {
    if (ebp == 0 || ebp < (uint32_t *)KERNEL_BASE || ebp == (uint32_t *)0xffffffff)
      break;
    pcs[i] = ebp[1];      // saved %eip
    ebp = (uint32_t *)ebp[0]; // saved %ebp
  }
  for (; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int holding(struct spinlock *lock)
{
  return lock->locked && lock->cpu == Get_Scheduler();
}

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void pushcli(void)
{
  int eflags;

  eflags = readeflags();
  cli();
  if (Get_Scheduler()->ncli == 0)
    Get_Scheduler()->intena = eflags & FL_IF;
  Get_Scheduler()->ncli += 1;
}

void popcli(void)
{
  if (readeflags() & FL_IF)
    panic("popcli - interruptible");
  if (--Get_Scheduler()->ncli < 0)
    panic("popcli");
  if (Get_Scheduler()->ncli == 0 && Get_Scheduler()->intena)
    sti();
}
