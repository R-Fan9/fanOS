#include "tss.h"
#include "C/stdint.h"
#include "C/string.h"
#include "gdt.h"

static tss_entry TSS;

// load TSS into TSR register
void flush_tss(uint16_t sel);

void tss_set_stack(uint16_t kernel_ss, uint16_t kernel_esp) {
  TSS.ss0 = kernel_ss;
  TSS.esp0 = kernel_esp;
}

void tss_init(uint32_t idx, uint16_t kernel_ss, uint16_t kernel_esp) {
  uint32_t base = (uint32_t)&TSS;

  uint8_t access = GDT_DESC_ACCESS;
  access |= GDT_DESC_EXEC_CODE;
  access |= GDT_DESC_MEMORY;
  access |= GDT_DESC_DPL;

  gdt_set_descriptor(idx, base, base + sizeof(tss_entry), access, 0);

  memset(&TSS, 0, sizeof(tss_entry));

  TSS.ss0 = kernel_ss;
  TSS.esp0 = kernel_esp;

  TSS.cs = 0x0B; // 0x0B = 0x08 + 0x03
  TSS.ss = 0x13; // 0x13 = 0x10 + 0x03
  TSS.es = 0x13;
  TSS.ds = 0x13;
  TSS.fs = 0x13;
  TSS.gs = 0x13;

  flush_tss(idx * sizeof(gdt_descriptor) + 3);
}

void flush_tss(uint16_t sel) { __asm__ __volatile("ltr %w0" : : "r"(sel)); }
