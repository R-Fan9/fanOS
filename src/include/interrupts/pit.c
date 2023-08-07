#include "pit.h"
#include "C/stdint.h"
#include "debug/display.h"
#include "hal/pic.h"
#include "ports/io.h"

static uint32_t pit_ticks = 0;

uint32_t get_tick_count() { return pit_ticks; }

void timer_irq0_handler() {
  pit_ticks++;
  send_pic_eoi(0);
}

void pit_set_channel_mode_frequency(uint8_t channel, int32_t freq,
                                    uint8_t mode) {
  if (channel > 2 || !freq) {
    return;
  }

  uint16_t divisor = (uint16_t)(1193181 / (uint16_t)freq);
  uint8_t cmd = 0;
  if (channel == 0) {
    cmd = (cmd & ~PIT_OCW_MASK_CHANNEL) | PIT_OCW_CHANNEL_0;
  } else if (channel == 1) {
    cmd = (cmd & ~PIT_OCW_MASK_CHANNEL) | PIT_OCW_CHANNEL_1;
  } else {
    cmd = (cmd & ~PIT_OCW_MASK_CHANNEL) | PIT_OCW_CHANNEL_2;
  }
  cmd = (cmd & ~PIT_OCW_MASK_MODE) | mode;
  cmd = (cmd & ~PIT_OCW_MASK_RL) | PIT_OCW_RL_DATA;

  outb(PIT_CMD, cmd);
  outb(PIT_CHANNEL_0 + channel, divisor & 0xFF);
  outb(PIT_CHANNEL_0 + channel, (divisor >> 8) & 0xFF);
  pit_ticks = 0;
}
