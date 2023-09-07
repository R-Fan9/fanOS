#include "C/stdint.h"
#include "C/stdio.h"
#include "C/stdlib.h"
#include "debug/display.h"

void userland(void) {
  clear_screen();

  print_string((uint8_t *)"Hello user land");

  while (1) {
  }
}
