#include "C/stdint.h"

#ifndef PIC_H
#define PIC_H

#define PIC_1_CMD 0x20
#define PIC_1_DATA 0x21

#define PIC_2_CMD 0xA0
#define PIC_2_DATA 0xA1

// IRQ 0 - 7 will be mapped to interrupts 0x20 - 0x27 (32 - 39)
#define NEW_IRQ_0 0x20

// IRQ 8 - 15 will be mapped to interrupts 0x28 - 0x2F (40 - 47)
#define NEW_IRQ_8 0x28

// "End of Interrupt" command
#define PIC_EOI 0x20

void send_pic_eoi(uint8_t irq);
void disable_pic(void);
void set_irq_mask(uint8_t irq);
void clear_irq_mask(uint8_t irq);
void remap_pic(void);

#endif // !PIC_H
