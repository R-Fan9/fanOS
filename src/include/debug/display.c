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
  return color;
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

void position_cursor(uint8_t x, uint8_t y){
  cursor_x = x;
  cursor_y = y;
}

void scroll_screen() {
  if (cursor_y < 25) {
    return;
  }

  uint8_t attribute = get_attribute();

  for (uint32_t i = 0; i < 80 * 24; i++) {
    framebuffer[i] = framebuffer[i + 80];
  }

  uint16_t blank = CT_WHT | (attribute << 8);
  for (uint32_t i = 24 * 80; i < 80 * 25; i++) {
    framebuffer[i] = blank;
  }

  cursor_y = 24;
}

void clear_screen() {
  uint8_t attribute = get_attribute();
  uint16_t blank = CT_WHT | (attribute << 8);

  for (uint32_t i = 0; i < 80 * 25; i++) {
    framebuffer[i] = blank;
  }

  cursor_x = 0;
  cursor_y = 0;
  display_cursor();
}

void print_char(uint8_t c) {

  uint8_t attribute = get_attribute();
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
    *loc = c | (attribute << 8);
    cursor_x++;
  }

  // move to a new line once exceeded the screen width
  if (cursor_x >= 80) {
    cursor_x = 0;
    cursor_y++;
  }

  scroll_screen();
  display_cursor();
}

void print_string(uint8_t *str) {
  for (; *str != '\0'; str++) {
    print_char(*str);
  }
}

// void print_hex(uint32_t hex){
//
// }

void print_dec(uint32_t num) {

  if (num == 0) {
    print_char('0');
    return;
  }

  uint8_t tmp[32];
  int32_t snum = num;
  int32_t i = 0;
  while (snum > 0) {
    tmp[i] = '0' + snum % 10;
    snum /= 10;
    i++;
  }
  tmp[i] = 0;

  uint8_t str[32];
  str[i--] = 0;
  int32_t j = 0;
  while (i >= 0) {
    str[i--] = tmp[j++];
  }

  print_string(str);
}
