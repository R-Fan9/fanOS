#include "display.h"
#include "C/ctype.h"
#include "C/stdint.h"
#include "ports/io.h"

static uint16_t *framebuffer = (uint16_t *)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

uint16_t get_cursor_location() { return cursor_y * MAX_ROW + cursor_x; }

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

void position_cursor(uint8_t x, uint8_t y) {
  cursor_x = x;
  cursor_y = y;
}

void scroll_screen() {
  if (cursor_y < MAX_COL) {
    return;
  }

  uint8_t attribute = get_attribute();

  for (uint32_t i = 0; i < MAX_ROW * (MAX_COL - 1); i++) {
    framebuffer[i] = framebuffer[i + MAX_ROW];
  }

  uint16_t blank = CT_WHT | (attribute << 8);
  for (uint32_t i = (MAX_COL - 1) * MAX_ROW; i < MAX_ROW * MAX_COL; i++) {
    framebuffer[i] = blank;
  }

  cursor_y = MAX_COL - 1;
}

void clear_screen() {
  uint8_t attribute = get_attribute();
  uint16_t blank = CT_WHT | (attribute << 8);

  for (uint32_t i = 0; i < MAX_ROW * MAX_COL; i++) {
    framebuffer[i] = blank;
  }

  cursor_x = 0;
  cursor_y = 0;
  display_cursor();
}

void print_char(uint8_t c) {

  uint16_t *loc;
  uint8_t attribute = get_attribute();

  // back space
  if (c == CT_BSP && (cursor_x || cursor_y)) {
    if (cursor_x) {
      cursor_x--;
    } else if (cursor_y) {
      cursor_y--;
      cursor_x = MAX_ROW - 1;
    }
    loc = framebuffer + get_cursor_location();
    *loc = CT_WHT | (attribute << 8);
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
    loc = framebuffer + get_cursor_location();
    *loc = c | (attribute << 8);
    cursor_x++;
  }

  // move to a new line once exceeded the screen width
  if (cursor_x >= MAX_ROW) {
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

void reverse_string(uint8_t last_idx, uint8_t *string) {
  uint8_t i = last_idx;
  uint8_t tmp;
  for (uint8_t j = 0; j < i; j++, i--) {
    tmp = string[j];
    string[j] = string[i];
    string[i] = tmp;
  }
}

void print_hex(uint32_t hex) {

  if (hex == 0) {
    print_string((uint8_t *)"0x0");
    return;
  }

  uint8_t hex_string[80];
  uint8_t *ascii_numbers = (uint8_t *)"0123456789ABCDEF";
  uint8_t nibble;
  uint8_t i = 0;

  while (hex > 0) {
    // Convert hex values to ascii string
    nibble = (uint8_t)hex & 0x0F;   // Get lowest 4 bits
    nibble = ascii_numbers[nibble]; // Hex to ascii
    hex_string[i] = nibble;         // Move ascii char into string
    hex >>= 4;                      // Shift right by 4 for next nibble
    i++;
  }

  // Add initial "0x" to front of hex string
  hex_string[i++] = 'x';
  hex_string[i++] = '0';
  hex_string[i] = '\0'; // Null terminate string

  i--; // Skip null byte
  reverse_string(i, hex_string);
  print_string(hex_string);
}

void print_dec(int32_t num) {
  if (num == 0) {
    print_char('0');
    return;
  }

  uint8_t dec_string[80];
  uint8_t i = 0;
  uint8_t negative = 0;

  if (num < 0) {
    negative = 1; // number is negative
    num = -num;
  }

  while (num > 0) {
    dec_string[i] = (num % 10) + '0'; // Store next digit as ascii
    num /= 10;                        // Remove last digit
    i++;
  }

  if (negative)
    dec_string[i++] = '-';

  dec_string[i] = '\0'; // null terminate

  i--;
  reverse_string(i, dec_string);
  print_string(dec_string);
}
