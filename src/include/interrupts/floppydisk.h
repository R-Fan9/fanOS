#include "C/stdint.h"
#include "hal/idt.h"

#ifndef FLOPPYDISK_H
#define FLOPPYDISK_H

#define DMA_BUFFER_BASE 0x1000;

enum FD_IO {
  FD_DOR = 0x3F2,  // digital output register
  FD_MSR = 0x3F4,  // main status register
  FD_FIFO = 0x3F5, // data register
  FD_CTRL = 0x3F7  // control register
};

enum FD_DOR_MASK {
  FD_DOR_MASK_DRIVE0 = 0,        // 00000000	= here for completeness sake
  FD_DOR_MASK_DRIVE1 = 1,        // 00000001
  FD_DOR_MASK_DRIVE2 = 2,        // 00000010
  FD_DOR_MASK_DRIVE3 = 3,        // 00000011
  FD_DOR_MASK_RESET = 4,         // 00000100
  FD_DOR_MASK_DMA = 8,           // 00001000
  FD_DOR_MASK_DRIVE0_MOTOR = 16, // 00010000
  FD_DOR_MASK_DRIVE1_MOTOR = 32, // 00100000
  FD_DOR_MASK_DRIVE2_MOTOR = 64, // 01000000
  FD_DOR_MASK_DRIVE3_MOTOR = 128 // 10000000
};

enum FD_MSR_MASK {
  FD_MSR_MASK_DRIVE1_POS_MODE = 1, // 00000001
  FD_MSR_MASK_DRIVE2_POS_MODE = 2, // 00000010
  FD_MSR_MASK_DRIVE3_POS_MODE = 4, // 00000100
  FD_MSR_MASK_DRIVE4_POS_MODE = 8, // 00001000
  FD_MSR_MASK_BUSY = 16,           // 00010000
  FD_MSR_MASK_DMA = 32,            // 00100000
  FD_MSR_MASK_DATAIO = 64,         // 01000000
  FD_MSR_MASK_DATAREG = 128        // 10000000
};

enum FD_CMD {
  FD_CMD_READ_TRACK = 2,
  FD_CMD_SPECIFY = 3,
  FD_CMD_CHECK_STAT = 4,
  FD_CMD_WRITE_SECT = 5,
  FD_CMD_READ_SECT = 6,
  FD_CMD_CALIBRATE = 7,
  FD_CMD_CHECK_INT = 8,
  FD_CMD_WRITE_DEL_S = 9,
  FD_CMD_READ_ID_S = 0xA,
  FD_CMD_READ_DEL_S = 0xC,
  FD_CMD_FORMAT_TRACK = 0xD,
  FD_CMD_SEEK = 0xF
};

enum FD_CMD_EXT {
  FD_CMD_EXT_SKIP = 0x20,      // 00100000
  FD_CMD_EXT_DENSITY = 0x40,   // 01000000
  FD_CMD_EXT_MULTITRACK = 0x80 // 10000000
};

// space between sectors on the physical disk
enum FD_GAP3_LENGTH {
  FD_GAP3_LENGTH_STD = 42,
  FD_GAP3_LENGTH_5_14 = 32,
  FD_GAP3_LENGTH_3_5 = 27
};

// bytes per sector, using the formula 2^n * 128
enum FD_SECTOR_DTL {
  FD_SECTOR_DTL_128 = 0,
  FD_SECTOR_DTL_256 = 1,
  FD_SECTOR_DTL_512 = 2,
  FD_SECTOR_DTL_1024 = 4
};

// initialize floppy disk controller
void fd_init(uint32_t drive);

// set current working drive
void fd_set_working_drive(uint8_t drive);

// get current working drive
void fd_get_working_drive();

// read a sector
uint8_t *fd_read_sector(int32_t sector_lba);

// converts a LBA address to CHS
void fd_lba_to_chs(int32_t lba, int32_t *head, int32_t *track, int32_t *sector);

__attribute__((interrupt)) void fd_irq6_handler(int_frame_t *frame);

#endif // !FLOPPYDISK_H
