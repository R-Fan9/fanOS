#include "pit.h"
#include "C/stdint.h"
#include "hal/pic.h"
#include "ports/io.h"

void timer_irq0_handler(void) {

  __asm__ __volatile__("add $12, %%esp\n"
                       "pusha\n");

  // increment tick count
  pit_ticks++;

  // signal "End of Interrupt" for IRQ0
  send_pic_eoi(0);

  __asm__ __volatile__("popa\n"
                       "iret\n");
}

void pit_set_channel_mode_frequency(uint8_t channel, int32_t freq,
                                    uint8_t mode) {
  if (channel > 2) {
    return;
  }

  uint16_t divisor = (uint16_t)1193181 / (uint16_t)freq;
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
