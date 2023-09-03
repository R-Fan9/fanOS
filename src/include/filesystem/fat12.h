#include "C/stdint.h"

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

void fat_display_image_size(uint32_t sector_count);
uint8_t *fat_find_image(uint8_t *img_name, uint8_t *buffer);
void fat_load_FAT(uint8_t *buffer);
void fat_load_root(uint8_t *buffer);
uint32_t fat_load_image(uint8_t *buffer, uint8_t *fat_buffer,
                        uint16_t img_cluster);
void fat_mount();
void fat_init();

#endif // !FAT12_H
