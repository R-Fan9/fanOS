#include "file.h"
#include "C/stdint.h"
#include "debug/display.h"

#define DEVICE_MAX 26

static PFILESYSTEM file_system[DEVICE_MAX];

FILE invalid_file();
uint8_t *get_filename(uint8_t *fname);
uint8_t get_device(uint8_t *fname);
FILE vol_open_file_impl(uint8_t *fname, uint8_t device);

void vol_read_file(PFILE file, uint8_t *buffer) {
  if (file) {
    PFILESYSTEM system = file_system[file->device_id - 'a'];
    if (system) {
      system->read(file, buffer);
    }
  }
}

void vol_close_file(PFILE file) {
  if (file) {
    PFILESYSTEM system = file_system[file->device_id - 'a'];
    if (system) {
      system->close(file);
    }
  }
}

void vol_register_filesystem(PFILESYSTEM system, uint32_t device_id) {
  if (device_id > DEVICE_MAX) {
    return;
  }
  file_system[device_id] = system;
}

void vol_register_filesystem_by_id(uint32_t device_id) {
  if (device_id > DEVICE_MAX) {
    return;
  }
  file_system[device_id] = 0;
}

void vol_unregister_filesystem(PFILESYSTEM system) {
  for (uint8_t i = 0; i < DEVICE_MAX; i++) {
    if (file_system[i] == system) {
      file_system[i] = 0;
    }
  }
}

FILE vol_open_file(const uint8_t *fname) {
  if (!fname) {
    return invalid_file();
  }

  uint8_t *filename = get_filename((uint8_t *)fname);
  uint8_t device = get_device((uint8_t *)fname);
  return vol_open_file_impl(filename, device);
}

FILE vol_open_file_impl(uint8_t *fname, uint8_t device) {
  PFILESYSTEM system = file_system[device - 'a'];
  if (!system) {
    return invalid_file();
  }

  FILE file = system->open(fname);
  file.device_id = device;
  return file;
}

uint8_t *get_filename(uint8_t *fname) {
  if (fname[1] == ':') {
    return fname + 2;
  }
  return fname;
}

uint8_t get_device(uint8_t *fname) {
  if (fname[1] == ':') {
    return fname[0];
  }
  // default device - 'a'
  return 'a';
}

FILE invalid_file() {
  FILE file;
  file.flags = FS_INVAILD;
  return file;
}
