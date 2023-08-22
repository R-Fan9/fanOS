#include "hal/idt.h"

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA 0x60
#define KEYBOARD_STATUS 0x64

__attribute__((interrupt)) void keyboard_irq1_handler(int_frame_t *frame);

#endif // KEYBOARD_H
