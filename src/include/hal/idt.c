#include "idt.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "interrupts/pit.h"

static struct idt_descriptor idt[IDT_SIZE];
static struct idtr idt_ptr;

__attribute__((interrupt)) void default_excp_handler(int_frame_t *frame) {
  (void)frame;
  clear_screen();
  print_string((uint8_t *)"DEFAULT EXCEPTION HANDLER - NO ERROR CODE");
  sleep(200);
}

__attribute__((interrupt)) void
default_excp_handler_err_code(int_frame_t *frame, uint32_t error_code) {
  (void)frame;
  clear_screen();
  print_string((uint8_t *)"DEFAULT EXCEPTION HANDLER - ERROR CODE:");
  print_hex(error_code);
  sleep(200);
}

__attribute__((interrupt)) void default_int_handler(int_frame_t *frame) {
  (void)frame;
  clear_screen();
  print_string((uint8_t *)"DEFAULT INTERRUPT HANDLER");
  sleep(200);
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

  // set up exception handlers first (ISRs 0-31)
  for (uint8_t entry = 0; entry < 32; entry++) {
    if (entry == 8 || entry == 10 || entry == 11 || entry == 12 ||
        entry == 13 || entry == 14 || entry == 17 || entry == 21) {
      // Exception takes an error code
      idt_set_descriptor(entry, default_excp_handler_err_code, TRAP_GATE_FLAGS);
    } else {
      // Exception does not take an error code
      idt_set_descriptor(entry, default_excp_handler, TRAP_GATE_FLAGS);
    }
  }

  // set up regular interrupts (ISRs 32-255)
  for (uint16_t entry = 32; entry < IDT_SIZE; entry++) {
    idt_set_descriptor(entry, default_int_handler, INT_GATE_FLAGS);
  }

  __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}
