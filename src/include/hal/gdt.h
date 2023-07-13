#include "C/stdint.h"

#define GDT_SIZE 8192

struct gdt_descriptor {
  // bits 0 - 15 of segment limt
  uint16_t limit;

  // bits 0 - 23 of base address
  uint16_t base_low;
  uint8_t base_mid;

  // descriptor bits flags and granularity
  uint8_t flags;
  uint8_t grand;

  // bits 24 - 32 of base address
  uint8_t base_high;
};

struct gdtr {
  // size of gdt
  uint16_t limit;

  // base address of gdt
  uint32_t base;
};

static struct gdt_descriptor   gdt[GDT_SIZE];
// static struct gdtr             gdt_ptr;
//
// void gdt_install();
void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand);


