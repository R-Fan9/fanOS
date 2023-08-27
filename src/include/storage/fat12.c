#include "fat12.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "floppydisk.h"
#include "interrupts/pit.h"

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
  print_hex(cluster);
  print_char(' ');

  uint32_t sector_count = 0;
  while (cluster < 0x0FF0) {
    uint32_t img_sector = fd_chs_to_lba(cluster) + data_sector;

    for (uint32_t i = 0; i < bpbSectorsPerCluster; i++) {
      uint8_t *sector = fd_read_sector(img_sector + i);
      if (sector) {
        memcpy(&buffer[i * bpbBytesPerSector], sector, bpbBytesPerSector);
      }
    }
    sector_count++;

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
    print_hex(cluster);
    print_char(' ');
    sleep(500);
  }

  return sector_count;
}
