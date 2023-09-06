#include "C/stdint.h"

#ifndef TSS_H
#define TSS_H

typedef struct _tss_entry {
  uint32_t prev_tss; // the previous TSS
  uint32_t esp0;     // the stack pointer to load when changing to kernel mode
  uint32_t ss0;      // the stack segment to load when changing to kernel mode
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldt;
  uint16_t trap;
  uint16_t iomap;
} __attribute__((packed)) tss_entry;

void tss_set_stack(uint16_t kernel_ss, uint16_t kernel_esp);
void tss_init(uint32_t idx, uint16_t kernel_ss, uint16_t kernel_esp);

#endif // !TSS_H
