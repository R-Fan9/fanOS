#include "C/stdint.h"

#ifndef DISPLAY_H
#define DISPLAY_H

#define BG_COLOR 0x0
#define FG_COLOR 0xF
#define CURSOR_CMD 0x3D4
#define CURSOR_DATA 0x3D5

void print_char(uint8_t c);
void print_string(uint8_t *str);
void clear_screen();
// void print_hex(uint32_t hex);
// void print_dec(int32_t number);

#endif // !PRINT_TYPES_H
