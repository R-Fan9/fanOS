#include "gdt.h"

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit,
                        uint8_t access, uint8_t grand) {
  if (i > GDT_SIZE) {
    return;
  }

  gdt[i].base_low = base & 0xFFFF;
  gdt[i].base_mid = (base >> 16) & 0xFF;
  gdt[i].base_high = (base >> 24) & 0xFF;
  gdt[i].limit = limit & 0xFFFF;
  gdt[i].flags = access;
  gdt[i].grand = (limit >> 16) & 0x0F;
  gdt[i].grand |= grand & 0xF0;
}
