#include "floppydisk.h"
#include "C/stdbool.h"
#include "C/stdint.h"
#include "debug/display.h"
#include "hal/dma.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/pit.h"
#include "ports/io.h"

void fd_control_motor(bool on);
void fd_wait_irq6();
void fd_write_dor(uint8_t val);
uint8_t fd_read_status();
void fd_send_command(uint8_t cmd);
uint8_t fd_read_data();
void fd_write_ccr(uint8_t val);
void fd_read_sector_impl(uint8_t head, uint8_t track, uint8_t sector);
void fd_drive_data(uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma);
int32_t fd_calibrate(uint32_t drive);
void fd_check_int(uint32_t *st0, uint32_t *cy1);
int32_t fd_seek(uint32_t cy1, uint32_t head);
void fd_disable_controller();
void fd_enable_controller();
void fd_reset();

static uint8_t current_drive = 0;

// set when IRQ6 fires
static uint8_t fd_irq6_stat = 0;

const uint32_t FD_SECTORS_PER_TRACK = 18;

void fd_control_motor(bool on) {
  if (current_drive > 3) {
    return;
  }

  uint32_t motor = 0;
  switch (current_drive) {
  case 0:
    motor = FD_DOR_MASK_DRIVE0_MOTOR;
    break;
  case 1:
    motor = FD_DOR_MASK_DRIVE1_MOTOR;
    break;
  case 2:
    motor = FD_DOR_MASK_DRIVE2_MOTOR;
    break;
  case 3:
    motor = FD_DOR_MASK_DRIVE3_MOTOR;
    break;
  }

  if (on == true) {
    fd_write_dor(motor | FD_DOR_MASK_RESET | FD_DOR_MASK_DMA);
  } else {
    fd_write_dor(FD_DOR_MASK_RESET);
  }

  sleep(20);
}

// wait for IRQ6 to finish
void fd_wait_irq6() {
  while (!fd_irq6_stat)
    ;
  fd_irq6_stat = 0;
}

// write to FDC digital output register
void fd_write_dor(uint8_t val) { outb(FD_DOR, val); }

// read FDC status
uint8_t fd_read_status() { return inb(FD_MSR); }

// send command to FDC data register
void fd_send_command(uint8_t cmd) {
  for (uint32_t i = 0; i < 500; i++) {
    if (fd_read_status() & FD_MSR_MASK_DATAREG) {
      return outb(FD_FIFO, cmd);
    }
  }
}

// read data from FDC data register
uint8_t fd_read_data() {
  for (uint32_t i = 0; i < 500; i++) {
    if (fd_read_status() & FD_MSR_MASK_DATAREG) {
      return inb(FD_FIFO);
    }
  }
  return 0;
}

void fd_write_ccr(uint8_t val) { outb(FD_CTRL, val); }

// pass controlling information to the FDC about the drive connected to it
void fd_drive_data(uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma) {
  uint32_t data = 0;

  fd_send_command(FD_CMD_SPECIFY);
  data = ((stepr & 0xF) << 4) | (unloadt & 0xF);
  fd_send_command(data);
  data = (loadt << 1) | ((dma == true) ? 1 : 0);
  fd_send_command(data);
}

// position the read/write head to cylinder 0
int32_t fd_calibrate(uint32_t drive) {
  uint32_t st0, cy1;
  if (drive > 3) {
    return -1;
  }

  fd_control_motor(true);

  for (uint32_t i = 1; i < 10; i++) {
    fd_send_command(FD_CMD_CALIBRATE);
    fd_send_command(drive);
    fd_wait_irq6();
    fd_check_int(&st0, &cy1);

    // cylinder 0 found
    if (!cy1) {
      fd_control_motor(false);
      return 0;
    }
  }

  fd_control_motor(false);
  return -1;
}

// check the state of FDC when an interrupt returns
void fd_check_int(uint32_t *st0, uint32_t *cy1) {
  fd_send_command(FD_CMD_CHECK_INT);
  *st0 = fd_read_data();
  *cy1 = fd_read_data();
}

// move the read/write head to a specific cylinder
int32_t fd_seek(uint32_t cy1, uint32_t head) {
  uint32_t st0, cy10;

  if (current_drive > 3) {
    return -1;
  }

  for (uint32_t i = 1; i < 10; i++) {
    fd_send_command(FD_CMD_SEEK);
    fd_send_command((head << 2) | current_drive);
    fd_send_command(cy1);
    fd_wait_irq6();
    fd_check_int(&st0, &cy10);

    // found the cylinder
    if (cy1 == cy10) {
      return 0;
    }
  }

  return -1;
}

void fd_disable_controller() { fd_write_dor(0); }
void fd_enable_controller() {
  fd_write_dor(FD_DOR_MASK_RESET | FD_DOR_MASK_DMA);
}
void fd_reset() {

  uint32_t st0, cy1;

  fd_disable_controller();
  fd_enable_controller();
  fd_wait_irq6();

  // send check interrupt command to all drivers
  for (uint32_t i = 0; i < 4; i++) {
    fd_check_int(&st0, &cy1);
  }

  // transfer speed 500KB/s
  fd_write_ccr(0);

  // step rate=3ms, unload time=240ms, load time=16ms
  fd_drive_data(3, 16, 240, true);

  fd_calibrate(current_drive);
}

__attribute__((interrupt)) void fd_irq6_handler(int_frame_t *frame) {
  (void)frame;
  fd_irq6_stat = 1;
  send_pic_eoi(6);
}

void fd_set_working_drive(uint8_t drive) {
  if (drive < 4) {
    current_drive = drive;
  }
}

void fd_lba_to_chs(int32_t lba, int32_t *head, int32_t *track,
                   int32_t *sector) {
  *head = (lba % (FD_SECTORS_PER_TRACK * 2)) / (FD_SECTORS_PER_TRACK);
  *track = lba / (FD_SECTORS_PER_TRACK * 2);
  *sector = lba % FD_SECTORS_PER_TRACK + 1;
}

uint32_t fd_chs_to_lba(uint16_t cluster) {
  return (cluster - 2) * bpbSectorsPerCluster;
}

void fd_read_sector_impl(uint8_t head, uint8_t track, uint8_t sector) {
  uint32_t next_sector = sector + 1;
  if (next_sector >= FD_SECTORS_PER_TRACK) {
    next_sector = FD_SECTORS_PER_TRACK;
  }

  uint32_t st0, cy1;

  dma_set_read(FDC_DMA_CHANNEL);
  fd_send_command(FD_CMD_READ_SECT | FD_CMD_EXT_MULTITRACK | FD_CMD_EXT_SKIP |
                  FD_CMD_EXT_DENSITY);
  fd_send_command(head << 2 | current_drive);
  fd_send_command(track);
  fd_send_command(head);
  fd_send_command(sector);
  fd_send_command(FD_SECTOR_DTL_512);
  fd_send_command(next_sector);
  fd_send_command(FD_GAP3_LENGTH_3_5);
  fd_send_command(0xFF);

  // wait for IRQ6
  fd_wait_irq6();

  // read 7 bytes status info
  for (uint32_t i = 0; i < 7; i++) {
    fd_read_data();
  }

  // let FDC know that the interrupt is handled
  fd_check_int(&st0, &cy1);
}

uint8_t *fd_read_sector(int32_t sector_lba) {
  if (current_drive > 3) {
    return 0;
  }

  int32_t head = 0, track = 0, sector = 1;
  fd_lba_to_chs(sector_lba, &head, &track, &sector);
  fd_control_motor(true);
  if (fd_seek(track, head) != 0) {
    return 0;
  }
  fd_read_sector_impl(head, track, sector);
  fd_control_motor(false);
  return (uint8_t *)DMA_BUFFER;
}

void fd_init(uint32_t drive) {

  // initialize DMA for floppy disk controller
  dma_init_floppy((uint8_t *)DMA_BUFFER, 512);

  fd_set_working_drive(drive);

  fd_reset();

  // step rate = 13ms, load time = 1ms, unload time = 0xFms, DMA
  fd_drive_data(13, 1, 0xF, true);
}
