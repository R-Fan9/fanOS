#include "C/stdint.h"

#ifndef FILE_H
#define FILE_H

// file flags
#define FS_FILE 0
#define FS_DIRECTORY 1
#define FS_INVAILD 2

typedef struct _FILE {

  uint8_t name[32];
  uint32_t flags;
  uint32_t size;
  uint32_t id;
  uint32_t end;
  uint32_t position;
  uint32_t current_cluster;
  uint32_t device_id;
} FILE, *PFILE;

typedef struct _FILE_SYSTEM {

  uint8_t name[8];
  FILE (*directory)(const uint8_t *dirname);
  void (*mount)();
  void (*read)(PFILE file, uint8_t *buffer, uint32_t size);
  void (*close)(PFILE);
  FILE (*open)(const uint8_t *filename);
} FILESYSTEM, *PFILESYSTEM;

FILE vol_open_file(const uint8_t *fname);
void vol_read_file(PFILE file, uint8_t *buffer, uint32_t size);
void vol_close_file(PFILE file);
void vol_register_filesystem(PFILESYSTEM system, uint32_t device_id);
void vol_register_filesystem_by_id(uint32_t device_id);
void vol_unregister_filesystem(PFILESYSTEM system);

#endif // !FILE_H
