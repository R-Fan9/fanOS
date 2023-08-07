#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA 0x60
#define KEYBOARD_STATUS 0x64

void keyboard_irq1_handler();

#endif // KEYBOARD_H
