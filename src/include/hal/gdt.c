#include "gdt.h"
#include "C/string.h"
#include "debug/display.h"

static gdt_descriptor gdt[GDT_SIZE];
static gdtr gdt_ptr;

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit,
                        uint8_t access, uint8_t grand) {
  if (i > GDT_SIZE) {
    return;
  }

  memset(&gdt[i], 0, sizeof(gdt_descriptor));

  gdt[i].limit = limit & 0xFFFF;
  gdt[i].base_low = base & 0xFFFF;
  gdt[i].base_mid = (base >> 16) & 0xFF;
  gdt[i].flags = access;
  gdt[i].grand = (limit >> 16) & 0x0F;
  gdt[i].grand |= grand & 0xF0;
  gdt[i].base_high = (base >> 24) & 0xFF;
}

void gdt_init() {

  // set up gdtr
  gdt_ptr.limit = sizeof(gdt_descriptor) * GDT_SIZE - 1;
  gdt_ptr.base = (uint32_t)&gdt;

  uint8_t default_code_seg_access = GDT_DESC_READWRITE;
  default_code_seg_access |= GDT_DESC_EXEC_CODE;
  default_code_seg_access |= GDT_DESC_CODEDATA;
  default_code_seg_access |= GDT_DESC_MEMORY;

  uint8_t default_data_seg_access = GDT_DESC_READWRITE;
  default_data_seg_access |= GDT_DESC_CODEDATA;
  default_data_seg_access |= GDT_DESC_MEMORY;

  uint8_t user_code_seg_access = GDT_DESC_READWRITE;
  user_code_seg_access |= GDT_DESC_EXEC_CODE;
  user_code_seg_access |= GDT_DESC_CODEDATA;
  user_code_seg_access |= GDT_DESC_MEMORY;
  user_code_seg_access |= GDT_DESC_DPL;

  uint8_t user_data_seg_access = GDT_DESC_READWRITE;
  user_data_seg_access |= GDT_DESC_CODEDATA;
  user_data_seg_access |= GDT_DESC_MEMORY;
  user_data_seg_access |= GDT_DESC_DPL;

  uint8_t grand = GDT_GRAND_4K;
  grand |= GDT_GRAND_32BIT;
  grand |= GDT_GRAND_LIMITHI_MASK;

  // set null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);

  // set default code descriptor
  gdt_set_descriptor(1, 0, 0xFFFFFFFF, default_code_seg_access, grand);

  // set default data descriptor
  gdt_set_descriptor(2, 0, 0xFFFFFFFF, default_data_seg_access, grand);

  // set user code descriptor
  gdt_set_descriptor(3, 0, 0xFFFFFFFF, user_code_seg_access, grand);

  // set user data descriptor
  gdt_set_descriptor(4, 0, 0xFFFFFFFF, user_data_seg_access, grand);

  __asm__ __volatile__("lgdt %0" : : "m"(gdt_ptr));
}
