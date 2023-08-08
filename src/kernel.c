#include "C/ctype.h"
#include "C/stdint.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"
#include "debug/display.h"

uint8_t *logo = (uint8_t *)"\
    __  _______  _____\n\
   /  |/  / __ \\/ ___/\n\
  / /|_/ / / / /\\__ \\ Microcomputer Operating System \n\
 / /  / / /_/ /___/ / -------------------------------\n\
/_/  /_/\\____//____/  \n\0";

int main(void)
{
  clear_screen();

  print_dec(1000);
  print_hex(0xFF);
  // print_string(logo);

  // set up Global Descritor Table
  // gdt_init();

  // set up interrupts
  idt_init();
  //
  // TODO - set up exception handlers (i.e, divide by 0, page fault..)
  //
  // mask off all hardware interrupts, disable PIC
  disable_pic();

  // remap PIC IRQ interrupts (IRQ0 starts at 0x20)
  pic_init();

  // add ISRs for PIC hardware interrupts
  // idt_set_descriptor(0x20, timer_irq0_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x21, (uint32_t)keyboard_irq1_handler, INT_GATE_FLAGS);

  // enable PIC IRQ interrupts after setting up their descriptors
  // clear_irq_mask(0); // enable timer - IRQ
  clear_irq_mask(1);

  // set default PIT Timer IRQ0 rate - ~1000hz
  // 1193182 MHZ / 1193 = ~1000
  // pit_set_channel_mode_frequency(0, 1000, PIT_OCW_MODE_RATEGEN);

  // enable all interrupts
  __asm__ __volatile__("sti");

  while (1)
    __asm__("hlt\n\t");

  return 0;
}
