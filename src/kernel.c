#include "C/ctype.h"
#include "C/stdint.h"
#include "debug/display.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"

uint8_t *logo = (uint8_t *)"\
    __  _______  _____\n\
   /  |/  / __ \\/ ___/\n\
  / /|_/ / / / /\\__ \\ Microcomputer Operating System \n\
 / /  / / /_/ /___/ / -------------------------------\n\
/_/  /_/\\____//____/  \n\0";

void print_physical_memory_info(void);

int main(void) {
  clear_screen();

  // set up Global Descritor Table
  gdt_init();

  // set up interrupts
  idt_init();

  // TODO - set up exception handlers (i.e, divide by 0, page fault..)

  // mask off all hardware interrupts, disable PIC
  disable_pic();

  // remap PIC IRQ interrupts (IRQ0 starts at 0x20)
  pic_init();

  // add ISRs for PIC hardware interrupts
  idt_set_descriptor(0x20, timer_irq0_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x21, keyboard_irq1_handler, INT_GATE_FLAGS);

  // enable PIC IRQ interrupts after setting up their descriptors
  clear_irq_mask(0); // enable timer - IRQ0
  clear_irq_mask(1); // enable keyboard - IRQ1

  // set default PIT Timer IRQ0 rate - ~1000hz
  // 1193182 MHZ / 1193 = ~1000
  pit_set_channel_mode_frequency(0, 1000, PIT_OCW_MODE_RATEGEN);

  // enable all interrupts
  __asm__ __volatile__("sti");

  print_physical_memory_info();

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
  return 0;
}

void print_physical_memory_info(void) {

  typedef struct SMAP_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
  } __attribute__((packed)) SMAP_entry_t;

  uint32_t num_entries = *(uint32_t *)0x1000;
  SMAP_entry_t *entry = (SMAP_entry_t *)0x1004;

  for (uint32_t i = 0; i < num_entries; i++) {
    print_string((uint8_t *)"region: ");
    print_hex(i);
    print_string((uint8_t *)" start: ");
    print_hex(entry->base);
    print_string((uint8_t *)" length: ");
    print_hex(entry->length);
    print_string((uint8_t *)" type: ");
    print_hex(entry->type);
    print_char('\n');
    entry++;
  }
}
