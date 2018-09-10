#include "lapic.h"
#include "../apic.h"

uint32_t __attribute__((optimize("O0"))) localapic_read(uint32_t base, uint32_t reg)
{
  uint32_t volatile *localapic = (uint32_t volatile *)(base+reg);
  return *localapic;
}

void __attribute__((optimize("O0"))) localapic_write(uint32_t base, uint32_t reg, uint32_t value)
{
  uint32_t volatile *localapic = (uint32_t volatile *)(base+reg);
  *localapic = value;
  *(uint32_t*)(null_read_4byte) = ((uint32_t*)(base))[LAPIC_ID/0x4];
}

inline void localapic_write_with_mask(uint32_t base, uint32_t reg, uint32_t mask, uint32_t value)
{
  uint32_t volatile *localapic = (uint32_t volatile *)(base+reg);
  *localapic &= ~mask;
  *localapic |= value;
  *(uint32_t*)(null_read_4byte) = ((uint32_t*)(base))[LAPIC_ID/0x4];
}
