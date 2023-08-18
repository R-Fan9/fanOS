#include "keyboard.h"
#include "C/stdint.h"
#include "debug/display.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "keymap.h"
#include "ports/io.h"

__attribute__((interrupt)) void keyboard_irq1_handler(int_frame_t *frame) {
  (void)frame;
  uint8_t status = inb(KEYBOARD_STATUS);
  if (status & 0x01) {
    int8_t keycode = inb(KEYBOARD_DATA);
    if (keycode >= 0) {
      print_char(keyboard_map[(uint8_t)keycode]);
    }
  }
  send_pic_eoi(1);
}
