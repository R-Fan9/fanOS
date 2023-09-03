#include "C/stdint.h"

#ifndef BPB_H
#define BPB_H

// BIOS Parameter Block (BPB)
typedef struct _BIOS_PARAMATER_BLOCK {
  uint8_t OEM_name[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t number_of_fats;
  uint16_t num_root_entries;
  uint16_t num_sectors;
  uint8_t media;
  uint16_t sectors_per_fat;
  uint16_t sectors_per_track;
  uint16_t heads_per_cyl;
  uint32_t hidden_sectors;
  uint32_t long_sectors;

} __attribute__((packed)) BIOSPARAMATERBLOCK, *PBIOSPARAMATERBLOCK;

// BIOS Parameter Block (BPB) extended attributes
typedef struct _BIOS_PARAMATER_BLOCK_EXT {

  uint32_t sectors_per_fat; // sectors per fat
  uint16_t flags;           // flags
  uint16_t version;         // version
  uint32_t root_cluster;    // starting root directory
  uint16_t info_cluster;
  uint16_t backup_boot; // location of bootsector copy
  uint16_t reserved[6];

} __attribute__((packed)) BIOSPARAMATERBLOCKEXT, *PBIOSPARAMATERBLOCKEXT;

// boot sector
typedef struct _BOOT_SECTOR {

  uint8_t ignore[3];      // first 3 bytes are ignored (our jmp instruction)
  BIOSPARAMATERBLOCK bpb; // BPB structure
  BIOSPARAMATERBLOCKEXT bpbext; // extended BPB info
  uint8_t filler[448];          // needed to make struct 512 bytes

} __attribute((packed)) BOOTSECTOR, *PBOOTSECTOR;

#endif // !BPB_H
