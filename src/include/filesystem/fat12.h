#include "C/stdint.h"
#include "file.h"

#ifndef FAT12_H
#define FAT12_H

#define SECTOR_SIZE 512
#define SECTORS_PER_CLUSTER 1

typedef struct _MOUNT_INFO {

  uint32_t num_sectors;
  uint32_t fat_offset;
  uint32_t num_root_entries;
  uint32_t root_offset;
  uint32_t root_size;
  uint32_t fat_size;
  uint32_t fat_entry_size;
  uint32_t data_offset;

} MOUNT_INFO, *PMOUNT_INFO;

typedef struct _DIRECTORY {

  uint8_t filename[8]; // filename
  uint8_t ext[3];      // extension (8.3 filename format)
  uint8_t attrib;      // file attributes
  uint8_t reserved;
  uint8_t time_created_ms; // creation time
  uint16_t time_created;
  uint16_t date_created; // creation date
  uint16_t date_last_accessed;
  uint16_t first_cluster_hi_bytes;
  uint16_t last_mod_time; // last modification date/time
  uint16_t last_mod_date;
  uint16_t first_cluster; // first cluster of file data
  uint32_t filesize;      // size in bytes

} __attribute__((packed)) DIRECTORY, *PDIRECTORY;

FILE fat_parse_rootdir(const uint8_t *fname);
FILE fat_parse_subdir(FILE kfile, const uint8_t *fname);
void fat_read_file(PFILE file, uint8_t *buffer);
void fat_close_file(PFILE file);
FILE fat_open_file(const uint8_t *fname);
void fat_mount();
void fat_init();

void fat_display_file_size(uint32_t sector_count);
uint8_t *fat_find_image(uint8_t *img_name, uint8_t *buffer);
void fat_load_FAT(uint8_t *buffer);
void fat_load_root(uint8_t *buffer);
uint32_t fat_load_image(uint8_t *buffer, uint8_t *fat_buffer,
                        uint16_t img_cluster);
#endif // !FAT12_H
