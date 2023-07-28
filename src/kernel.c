#include "C/stdint.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/pit.h"

void main(void) {
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

  // enable PIC IRQ interrupts after setting up their descriptors
  clear_irq_mask(0); // enable timer - IRQ0
  

  pit_set_channel_mode_frequency(PIT_OCW_CHANNEL_0, 100, PIT_OCW_MODE_SQUAREWAVEGEN);

  char *vga = (char *)0xB8000;
  vga[0] = 'X';
  vga[1] = 0x28;
}
