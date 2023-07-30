#include "display.h"
#include "C/ctype.h"
#include "C/stdint.h"
#include "ports/io.h"

static uint16_t *framebuffer = (uint16_t *)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

uint16_t get_cursor_location() { return cursor_y * 80 + cursor_x; }
uint8_t get_attribute() {
  uint8_t color = (BG_COLOR << 4) | (FG_COLOR & 0x0F);
  return color << 8;
}

void display_cursor() {
  uint16_t cursor_loc = get_cursor_location();

  // tell the VGA board high cursor byte is being set
  outb(CURSOR_CMD, 0xE);
  // set the high cursor byte
  outb(CURSOR_DATA, cursor_loc >> 8);

  // tell the VGA board low cursor byte is being set
  outb(CURSOR_CMD, 0xF);
  // set the low cursor byte
  outb(CURSOR_DATA, (uint8_t)cursor_loc);
}

void position_cursor() {

  if (cursor_x > 80) {
    cursor_x = 0;
    cursor_y++;
  }

  if (cursor_y > 25) {
    cursor_y = 0;
  }
}

void clear_screen() {
  uint16_t attribute = get_attribute();

  for (uint32_t i = 0; i <= 80 * 25; i++) {
    framebuffer[i] = CT_WHT | attribute;
  }

  cursor_x = 0;
  cursor_y = 0;
  display_cursor();
}

void print_char(uint8_t c) {
  uint16_t attribute = get_attribute();
  uint16_t *loc = framebuffer + get_cursor_location();

  // back space
  if (c == CT_BSP && cursor_x) {
    *loc = CT_WHT | attribute;
    cursor_x--;
  }
  // tab character
  else if (c == CT_TAB) {
    cursor_x = (cursor_x + 8) & ~(8 - 1);
  }
  // carriage return
  else if (c == '\r') {
    cursor_x = 0;
  }
  // new line
  else if (c == '\n') {
    cursor_x = 0;
    cursor_y++;
  }
  // printable character
  else if (c >= CT_WHT) {
    *loc = c | attribute;
    cursor_x++;
  }

  position_cursor();
  display_cursor();
}

void print_string(uint8_t *str) {
  for (; *str != '\0'; str++) {
    print_char(*str);
  }
}
