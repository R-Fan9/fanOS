#include "gdt.h"
#include "C/stdint.h"
#include "C/string.h"

void gdt_install() { __asm__ volatile("lgdt %0" : : "m"(gdt_ptr)); }

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit,
                        uint8_t access, uint8_t grand) {
  if (i > GDT_SIZE) {
    return;
  }

  memset((void *)&gdt[i], 0, sizeof(struct gdt_descriptor));

  gdt[i].base_low = base & 0xFFFF;
  gdt[i].base_mid = (base >> 16) & 0xFF;
  gdt[i].base_high = (base >> 24) & 0xFF;
  gdt[i].limit = limit & 0xFFFF;
  gdt[i].flags = access;
  gdt[i].grand = (limit >> 16) & 0x0F;
  gdt[i].grand |= grand & 0xF0;
}

int gdt_init() {

  //! set up gdtr
  gdt_ptr.limit = (sizeof(struct gdt_descriptor) * GDT_SIZE) - 1;
  gdt_ptr.base = (uint32_t)&gdt[0];

  memset((void *)&gdt[0], 0, sizeof(struct gdt_descriptor) * GDT_SIZE - 1);

  //! set null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);

  //! set default code descriptor
  gdt_set_descriptor(1, 0, 0xFFFFFFFF,
                     GDT_DESC_READWRITE | GDT_DESC_EXEC_CODE |
                         GDT_DESC_CODEDATA | GDT_DESC_MEMORY,
                     GDT_GRAND_4K | GDT_GRAND_32BIT | GDT_GRAND_LIMITHI_MASK);

  //! set default data descriptor
  gdt_set_descriptor(2, 0, 0xFFFFFFFF,
                     GDT_DESC_READWRITE | GDT_DESC_CODEDATA | GDT_DESC_MEMORY,
                     GDT_GRAND_4K | GDT_GRAND_32BIT | GDT_GRAND_LIMITHI_MASK);

  //! install gdtr
  gdt_install();

  return 0;
}
