#include "pic.h"
#include "C/stdint.h"
#include "ports/io.h"

// send "End of Interrupt" command to signal a IRQ has been handled
void send_pic_eoi(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC_2_CMD, PIC_EOI);
  }
  outb(PIC_1_CMD, PIC_EOI);
}

// disable PIC setting all of its 8 bits to 1
void disable_pic() {
  outb(PIC_1_DATA, 0xFF);
  outb(PIC_2_DATA, 0xFF);
}

// disable a IRQ by setting bit 1 to its corresponding bit
void set_irq_mask(uint8_t irq) {
  uint16_t port = PIC_1_DATA;

  if (irq >= 8) {
    irq -= 8;
    port = PIC_2_DATA;
  }

  uint8_t curmask = inb(port);
  uint8_t value = curmask | (1 << irq);
  outb(port, value);
}

// enable a IRQ by setting bit 0 to its corresponding bit
void clear_irq_mask(uint8_t irq) {
  uint16_t port = PIC_1_DATA;

  if (irq >= 8) {
    irq -= 8;
    port = PIC_2_DATA;
  }

  uint8_t curmask = inb(port);
  uint8_t value = curmask & ~(1 << irq);
  outb(port, value);
}

// initialize PIC
void pic_init() {

  uint8_t pic_1_mask = inb(PIC_1_DATA);
  uint8_t pic_2_mask = inb(PIC_2_DATA);

  // ICW 1 - bit 0 = send up to ICW 4, bit 4 = initialize PIC
  outb(PIC_1_CMD, 0x11);
  outb(PIC_2_CMD, 0x11);

  // ICW 2 - align with base interrupt in IDT
  outb(PIC_1_DATA, 0x20);
  outb(PIC_2_DATA, 0x28);

  // ICW 3 - map PIC2 to IRQ2
  outb(PIC_1_DATA, 0x04); // bit # (0 based) - 0100 = bit 2 (IRQ2)
  outb(PIC_2_DATA, 0x02); // binary # for IRQ in PIC1, 0010 = 0x2 = 2

  // ICW 4 - set x86 mode
  outb(PIC_1_DATA, 0x01);
  outb(PIC_2_DATA, 0x01);

  // save current masks
  outb(PIC_1_DATA, pic_1_mask);
  outb(PIC_2_DATA, pic_2_mask);
}
