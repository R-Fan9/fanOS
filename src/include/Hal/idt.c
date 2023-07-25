#include "idt.h"
#include "C/string.h"

void idt_install() { __asm__ volatile("lidt %0" : : "m"(idt_ptr)); }

void idt_set_descriptor(uint32_t i, uint32_t base, uint16_t flags,
                        uint16_t sel) {
  if (i > IDT_SIZE) {
    return;
  }

  memset((void *)&idt[0], 0, sizeof(struct idt_descriptor));

  idt[i].base_low = base & 0xFFFF;
  idt[i].selector = sel;
  idt[i].reserved = 0;
  idt[i].flags = flags;
  idt[i].base_high = (base >> 16) & 0xFFFF;
}

int idt_init() {
  idt_ptr.limit = (sizeof(struct idt_descriptor) * IDT_SIZE) - 1;
  idt_ptr.base = (uint32_t)&idt[0];

  memset((void *)&idt[0], 0, sizeof(struct idt_descriptor) * IDT_SIZE - 1);

  idt_install();

  return 0;
}
