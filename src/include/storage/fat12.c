#include "fat12.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "floppydisk.h"
#include "interrupts/pit.h"

void fat_display_image_size(uint32_t sector_count) {
  print_string((uint8_t *)"image size: ");
  print_dec(sector_count);
  print_string((uint8_t *)" sectors, ");
  print_dec(sector_count * bpbBytesPerSector);
  print_string((uint8_t *)" bytes\n\n");
}

uint8_t *fat_find_image(uint8_t *img_name, uint8_t *buffer) {
  for (uint32_t i = 0; i < bpbRootEntries; i++) {
    if (strncmp(img_name, &buffer[i * 32], 11) == 0) {
      return &buffer[i * 32];
    }
  }
  return 0;
}

void fat_load_FAT(uint8_t *buffer) {
  uint32_t fat_sector = bpbReservedSectors;
  uint32_t fat_size = bpbNumberOfFATs * bpbSectorsPerFAT;
  for (uint32_t i = 0; i < fat_size; i++) {
    uint8_t *sector = fd_read_sector(fat_sector + i);
    if (sector) {
      memcpy(&buffer[i * bpbBytesPerSector], sector, bpbBytesPerSector);
    }
  }
}

void fat_load_root(uint8_t *buffer) {
  uint32_t root_sector =
      bpbNumberOfFATs * bpbSectorsPerFAT + bpbReservedSectors;
  uint32_t root_size = 32 * bpbRootEntries / bpbBytesPerSector;

  for (uint32_t i = 0; i < root_size; i++) {
    uint8_t *sector = fd_read_sector(root_sector + i);
    if (sector) {
      memcpy(&buffer[i * bpbBytesPerSector], sector, bpbBytesPerSector);
    }
  }
}

uint32_t fat_load_image(uint8_t *buffer, uint8_t *fat_buffer,
                        uint16_t img_cluster) {
  uint32_t root_sector =
      bpbNumberOfFATs * bpbSectorsPerFAT + bpbReservedSectors;
  uint32_t root_size = 32 * bpbRootEntries / bpbBytesPerSector;

  uint32_t data_sector = root_sector + root_size;

  uint16_t cluster = img_cluster;

  uint32_t sector_count = 0;
  while (cluster < 0x0FF0) {
    uint32_t img_sector = fd_chs_to_lba(cluster) + data_sector;

    uint32_t i = 0;
    for (; i < bpbSectorsPerCluster; i++) {
      uint8_t *sector = fd_read_sector(img_sector + i);

      if (sector) {
        memcpy(&buffer[(sector_count + i) * bpbBytesPerSector], sector,
               bpbBytesPerSector);
      }
    }
    sector_count += i;

    uint32_t fat_offset = cluster / 2 + cluster;
    uint16_t fat_entry = (uint16_t) * (uint16_t *)(fat_buffer + fat_offset);

    // even cluster
    if (cluster % 2 == 0) {
      // takes the lower 12 bits
      fat_entry &= 0xFFF;

      // odd cluster
    } else {
      // takes the higher 12 bits
      fat_entry >>= 4;
    }

    cluster = fat_entry;
  }

  return sector_count;
}
