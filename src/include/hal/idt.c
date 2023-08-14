#include "idt.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"

static struct idt_descriptor idt[IDT_SIZE];
static struct idtr idt_ptr;

void default_int_handler() {
  clear_screen();
  print_string((uint8_t *)"DEFAULT INTERRUPT HANDLER");
}

void idt_set_descriptor(uint32_t i, void *irq, uint16_t flags) {
  if (i > IDT_SIZE || !irq) {
    return;
  }

  idt[i].base_low = (uint32_t)irq & 0xFFFF;
  idt[i].selector = 0x08;
  idt[i].reserved = 0;
  idt[i].flags = (uint8_t)flags;
  idt[i].base_high = ((uint32_t)irq >> 16) & 0xFFFF;
}

void idt_init() {
  idt_ptr.limit = sizeof(struct idt_descriptor) * IDT_SIZE - 1;
  idt_ptr.base = (uint32_t)&idt;

  memset((void *)&idt, 0, sizeof(struct idt_descriptor) * IDT_SIZE - 1);

  for (uint16_t entry = 32; entry < IDT_SIZE; entry++) {
    idt_set_descriptor(entry, default_int_handler, INT_GATE_FLAGS);
  }

  __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}
