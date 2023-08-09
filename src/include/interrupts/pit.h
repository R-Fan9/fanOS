#include "C/stdint.h"

#ifndef PIT_H
#define PIT_H

#define PIT_CMD 0x43
#define PIT_CHANNEL_0 0x40
#define PIT_CHANNLE_1 0x41
#define PIT_CHANNEL_2 0x42

//-----------------------------------------------
//	Operational Command Bit masks
//-----------------------------------------------

#define PIT_OCW_MASK_BINCOUNT 1   // 00000001
#define PIT_OCW_MASK_MODE 0xE     // 00001110
#define PIT_OCW_MASK_RL 0x30      // 00110000
#define PIT_OCW_MASK_CHANNEL 0xC0 // 11000000

//-----------------------------------------------
//	Operational Command control bits
//-----------------------------------------------

//! Use when setting binary count mode
#define PIT_OCW_BINCOUNT_BINARY 0 // 0
#define PIT_OCW_BINCOUNT_BCD 1    // 1

//! Use when setting counter mode
#define PIT_OCW_MODE_TERMINALCOUNT 0   // 0000
#define PIT_OCW_MODE_ONESHOT 0x2       // 0010
#define PIT_OCW_MODE_RATEGEN 0x4       // 0100
#define PIT_OCW_MODE_SQUAREWAVEGEN 0x6 // 0110
#define PIT_OCW_MODE_SOFTWARETRIG 0x8  // 1000
#define PIT_OCW_MODE_HARDWARETRIG 0xA  // 1010

//! Use when setting data transfer
#define PIT_OCW_RL_LATCH 0      // 000000
#define PIT_OCW_RL_LSBONLY 0x10 // 010000
#define PIT_OCW_RL_MSBONLY 0x20 // 100000
#define PIT_OCW_RL_DATA 0x30    // 110000

//! Use when setting the counter we are working with
#define PIT_OCW_CHANNEL_0 0    // 00000000
#define PIT_OCW_CHANNEL_1 0x40 // 01000000
#define PIT_OCW_CHANNEL_2 0x80 // 10000000

uint32_t get_tick_count();
void timer_handler();
void pit_set_channel_mode_frequency(uint8_t channel, int32_t freq, uint8_t mode);

#endif // !PIC_H
