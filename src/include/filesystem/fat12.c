#include "fat12.h"
#include "C/stdint.h"
#include "C/string.h"
#include "bpb.h"
#include "debug/display.h"
#include "floppydisk.h"
#include "interrupts/pit.h"

static MOUNT_INFO mount_info;

void fat_display_image_size(uint32_t sector_count) {
  print_string((uint8_t *)"image size: ");
  print_dec(sector_count);
  print_string((uint8_t *)" sectors, ");
  print_dec(sector_count * SECTOR_SIZE);
  print_string((uint8_t *)" bytes\n\n");
}

uint8_t *fat_find_image(uint8_t *img_name, uint8_t *buffer) {
  for (uint32_t i = 0; i < mount_info.num_root_entries; i++) {
    if (strncmp(img_name, &buffer[i * 32], 11) == 0) {
      return &buffer[i * 32];
    }
  }
  return 0;
}

void fat_load_FAT(uint8_t *buffer) {
  for (uint32_t i = 0; i < mount_info.fat_size; i++) {
    uint8_t *sector = fd_read_sector(mount_info.fat_offset + i);
    if (sector) {
      memcpy(&buffer[i * SECTOR_SIZE], sector, SECTOR_SIZE);
    }
  }
}

void fat_load_root(uint8_t *buffer) {
  for (uint32_t i = 0; i < mount_info.root_size; i++) {
    uint8_t *sector = fd_read_sector(mount_info.root_offset + i);
    if (sector) {
      memcpy(&buffer[i * SECTOR_SIZE], sector, SECTOR_SIZE);
    }
  }
}

uint32_t fat_load_image(uint8_t *buffer, uint8_t *fat_buffer,
                        uint16_t img_cluster) {
  uint16_t cluster = img_cluster;
  uint32_t sector_count = 0;

  while (cluster < 0x0FF0) {
    uint32_t img_sector = fd_chs_to_lba(cluster) + mount_info.data_offset;

    for (uint32_t i = 0; i < SECTORS_PER_CLUSTER; i++) {
      uint8_t *sector = fd_read_sector(img_sector + i);

      if (sector) {
        memcpy(&buffer[(sector_count + i) * SECTOR_SIZE], sector, SECTOR_SIZE);
      }
    }
    sector_count += SECTORS_PER_CLUSTER;
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

void fat_mount() {
  PBOOTSECTOR boot_sector = (PBOOTSECTOR)fd_read_sector(0);
  BIOSPARAMATERBLOCK bpb = boot_sector->bpb;

  mount_info.num_sectors = bpb.num_sectors;
  mount_info.fat_offset = bpb.reserved_sectors;
  mount_info.fat_size = bpb.sectors_per_fat;
  mount_info.fat_entry_size = 8;
  mount_info.num_root_entries = bpb.num_root_entries;
  mount_info.root_offset =
      bpb.number_of_fats * bpb.sectors_per_fat + bpb.reserved_sectors;
  mount_info.root_size = bpb.num_root_entries * 32 / bpb.bytes_per_sector;
  mount_info.data_offset = mount_info.root_offset + mount_info.root_size;
}

void fat_init() {
  // mount file system
  fat_mount();
}
