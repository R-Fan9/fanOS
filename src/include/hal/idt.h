#include "C/stdint.h"

#ifndef IDT_H
#define IDT_H

#define IDT_SIZE 256
#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define INT_GATE_USER_FLAGS 0xEE

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
} __attribute__((packed));

struct idtr {
  // size of idt
  uint16_t limit;

  // base address of idt
  uint32_t base;
} __attribute__((packed));

void idt_set_descriptor(uint32_t i, uint32_t irq, uint16_t flags);
void idt_init();

#endif // !IDT_H
