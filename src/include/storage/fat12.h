#include "C/stdint.h"

#ifndef FAT12_H
#define FAT12_H

uint8_t *fat_find_image(uint8_t *img_name, uint8_t *buffer);
void fat_load_FAT(uint8_t *buffer);
void fat_load_root(uint8_t *buffer);
uint32_t fat_load_image(uint8_t *buffer, uint8_t *fat_buffer, uint16_t img_cluster);

#endif // !FAT12_H
