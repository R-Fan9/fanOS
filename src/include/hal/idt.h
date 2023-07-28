#include "C/stdint.h"

#ifndef IDT_H
#define IDT_H

#define IDT_SIZE 256
#define TRAP_GATE_FLAGS     0x8F    // P = 1, DPL = 00, S = 0, Type = 1111 (32bit trap gate)
#define INT_GATE_FLAGS      0x8E    // P = 1, DPL = 00, S = 0, Type = 1110 (32bit interrupt gate)
#define INT_GATE_USER_FLAGS 0xEE    // P = 1, DPL = 11, S = 0, Type = 1110 (32bit interrupt gate, called from PL 3)

struct idt_descriptor {
  // bits 0-16 of interrupt routine (IR) address
  uint16_t base_low;

  // code selector in gdt
  uint16_t selector;

  // reserved, should be 0
  uint8_t reserved;

  // bits flags
  uint8_t flags;

  // bits 16-32 of IR address
  uint16_t base_high;
};

struct idtr {
  // size of idt
  uint16_t limit;

  // base address of idt
  uint32_t base;
};

static struct idt_descriptor idt[IDT_SIZE];
static struct idtr idt_ptr;

void idt_install();
void idt_set_descriptor(uint32_t i, void *isr, uint16_t flags);
int idt_init();

#endif // !IDT_H
