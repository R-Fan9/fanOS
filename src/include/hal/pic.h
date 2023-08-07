#include "C/stdint.h"

#ifndef PIC_H
#define PIC_H

#define PIC_1_CMD 0x20
#define PIC_1_DATA 0x21

#define PIC_2_CMD 0xA0
#define PIC_2_DATA 0xA1

// "End of Interrupt" command
#define PIC_EOI 0x20

void send_pic_eoi(uint8_t irq);
void disable_pic();
void set_irq_mask(uint8_t irq);
void clear_irq_mask(uint8_t irq);
void pic_init();

#endif // !PIC_H
