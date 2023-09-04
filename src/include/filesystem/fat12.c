#include "fat12.h"
#include "C/stdbool.h"
#include "C/stdint.h"
#include "C/string.h"
#include "bpb.h"
#include "debug/display.h"
#include "floppydisk.h"
#include "interrupts/pit.h"

// FAT file system
FILESYSTEM filesystem;

// BIOS PARAMETER BLOCK (BPB) info
MOUNT_INFO mount_info;

// file allocation table (FAT)
uint8_t FAT[SECTOR_SIZE * 2];

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
  print_hex(cluster);
  print_char(' ');

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
    print_hex(cluster);
    print_char(' ');
  }

  return sector_count;
}

// locate file or directory in root directory
FILE fat_parse_rootdir(const uint8_t *fname) {
  FILE file;
  PDIRECTORY dir;

  uint8_t dos_fname[12];

  // TODO - convert file name to 8.3 DOS file name
  memcpy(dos_fname, fname, 11);
  dos_fname[11] = 0;

  // 14 sectors per directory
  for (uint32_t i = 0; i < 14; i++) {
    dir = (PDIRECTORY)fd_read_sector(mount_info.root_offset + i);

    // 16 entries per sector
    for (uint32_t i = 0; i < 16; i++) {

      if (strncmp(dos_fname, dir->filename, 11) == 0) {
        strcpy(file.name, fname);
        file.id = 0;
        file.current_cluster = dir->first_cluster;
        file.size = dir->filesize;
        file.end = 0;

        if (dir->attrib == 0x10) {
          file.flags = FS_DIRECTORY;
        } else {
          file.flags = FS_FILE;
        }
        return file;
      }
      dir++;
    }
  }

  file.flags = FS_INVAILD;
  return file;
}

// read from a file
void fat_read_file(PFILE file, uint8_t *buffer) {
  if (!file) {
    return;
  }

  uint32_t cur_cluster = file->current_cluster;
  print_hex(cur_cluster);
  print_char(' ');

  uint32_t data_sector = fd_chs_to_lba(cur_cluster) + mount_info.data_offset;
  uint8_t *sector = fd_read_sector(data_sector);
  memcpy(buffer, sector, SECTOR_SIZE);

  uint32_t fat_offset = (cur_cluster / 2) + cur_cluster;
  uint32_t fat_sector = fat_offset / SECTOR_SIZE + 1;
  uint32_t fat_entry_offset = fat_offset % SECTOR_SIZE;

  // read 1st FAT sector
  sector = fd_read_sector(fat_sector);
  memcpy(FAT, sector, SECTOR_SIZE);

  // read 2nd FAT sector
  sector = fd_read_sector(fat_sector + 1);
  memcpy(FAT + SECTOR_SIZE, sector, SECTOR_SIZE);

  uint16_t next_cluster = *(uint16_t *)&FAT[fat_entry_offset];

  // test if the cluster is odd or even
  if (cur_cluster % 2 == 0) {
    // even cluster -> takes the lower half 12 bits
    next_cluster &= 0xFFF;
  } else {
    // odd cluster -> takes the higher half 12 bits
    next_cluster >>= 4;
  }

  // test for end of file
  if (next_cluster >= 0xFF8) {
    file->end = 1;
    return;
  }

  // test for file corruption
  if (next_cluster == 0) {
    file->end = 1;
    return;
  }

  file->current_cluster = next_cluster;
}

// locate file or directory in subdirectory
FILE fat_parse_subdir(FILE kfile, const uint8_t *fname) {
  FILE file;
  PDIRECTORY dir;

  uint8_t dos_fname[12];

  // TODO - convert file name to 8.3 DOS file name
  memcpy(dos_fname, fname, 11);
  dos_fname[11] = 0;

  while (!kfile.end) {
    uint8_t buffer[SECTOR_SIZE];
    fat_read_file(&kfile, buffer);
    dir = (PDIRECTORY)buffer;

    // 16 entries per sector
    for (uint32_t i = 0; i < 16; i++) {

      if (strncmp(dos_fname, dir->filename, 11) == 0) {
        strcpy(file.name, fname);
        file.id = 0;
        file.current_cluster = dir->first_cluster;
        file.size = dir->filesize;
        file.end = 0;

        if (dir->attrib == 0x10) {
          file.flags = FS_DIRECTORY;
        } else {
          file.flags = FS_FILE;
        }
        return file;
      }
      dir++;
    }
  }

  file.flags = FS_INVAILD;
  return file;
}

// opens a file
FILE fat_open_file(const uint8_t *path) {
  FILE curdir;
  uint8_t *p = 0;
  bool rootdir = true;

  // any '\'s in the file path
  p = strchr(path, '\\');
  if (!p) {

    // nope, must be in root directory
    curdir = fat_parse_rootdir(path);
    if (curdir.flags == FS_FILE)
      return curdir;

    FILE ret;
    ret.flags = FS_INVAILD;
    return ret;
  }

  // go the next character after first '/'
  p++;
  while (p) {

    uint8_t *pathname = 0;
    uint32_t i = 0;
    while (*p != '\\' && *p != '\0') {
      pathname[i++] = *p++;
    }

    if (rootdir && pathname) {
      curdir = fat_parse_rootdir(pathname);
      rootdir = false;
    } else if (pathname) {
      curdir = fat_parse_subdir(curdir, pathname);
    }

    if (curdir.flags == FS_INVAILD) {
      break;
    }

    if (curdir.flags == FS_FILE) {
      return curdir;
    }

    p = strchr(p, '\\');
    if (p)
      p++;
  }

  FILE ret;
  ret.flags = FS_INVAILD;
  return ret;
}

// close a file
void fat_close_file(PFILE file) {
  if (file) {
    file->flags = FS_INVAILD;
  }
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
  strcpy(filesystem.name, (uint8_t *)"FAT12");
  filesystem.directory = fat_parse_rootdir;
  filesystem.mount = fat_mount;
  filesystem.read = fat_read_file;
  filesystem.close = fat_close_file;
  filesystem.open = fat_open_file;

  // register file system to volume manager
  vol_register_filesystem(&filesystem, 0);

  // mount file system
  fat_mount();
}
