#include "gdt.h"
#include "C/string.h"
#include "debug/display.h"

static struct gdt_descriptor gdt[GDT_SIZE];
static struct gdtr gdt_ptr;

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit,
                        uint8_t access, uint8_t grand) {
  if (i > GDT_SIZE) {
    return;
  }

  memset(&gdt[i], 0, sizeof(struct gdt_descriptor));

  gdt[i].base_low = base & 0xFFFF;
  gdt[i].base_mid = (base >> 16) & 0xFF;
  gdt[i].base_high = (base >> 24) & 0xFF;
  gdt[i].limit = limit & 0xFFFF;
  gdt[i].flags = access;
  gdt[i].grand = (limit >> 16) & 0x0F;
  gdt[i].grand |= grand & 0xF0;
}

void gdt_init() {

  // set up gdtr
  gdt_ptr.limit = sizeof(struct gdt_descriptor) * GDT_SIZE - 1;
  gdt_ptr.base = (uint32_t)&gdt;

  // set null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);

  uint8_t code_seg_access = GDT_DESC_READWRITE;
  code_seg_access |= GDT_DESC_EXEC_CODE;
  code_seg_access |= GDT_DESC_CODEDATA;
  code_seg_access |= GDT_DESC_MEMORY;

  uint8_t data_seg_access = GDT_DESC_READWRITE;
  data_seg_access |= GDT_DESC_CODEDATA;
  data_seg_access |= GDT_DESC_MEMORY;

  uint8_t grand = GDT_GRAND_4K;
  grand |= GDT_GRAND_32BIT;
  grand |= GDT_GRAND_LIMITHI_MASK;

  // set default code descriptor
  gdt_set_descriptor(1, 0, 0xFFFFFFFF, code_seg_access, grand);

  // set default data descriptor
  gdt_set_descriptor(2, 0, 0xFFFFFFFF, data_seg_access, grand);

  __asm__ __volatile__("lgdt %0" : : "m"(gdt_ptr));
}
