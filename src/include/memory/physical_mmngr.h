#include "C/stdint.h"

#ifndef PMM_H
#define PMM_H

typedef uint32_t physical_addr;

// 8 blocks per byte
#define BLOCKS_PER_BYTE 8

// block size (4k)
#define BLOCK_SIZE 4096

// block alignment
#define BLOCK_ALIGN PMMNGR_BLOCK_SIZE

void pmmngr_display_blocks();
void pmmngr_init_region(physical_addr base, uint32_t size);
void pmmngr_deinit_region(physical_addr base, uint32_t size);
physical_addr *pmmngr_alloc_block();
physical_addr *pmmngr_alloc_blocks(uint32_t blocks);
void pmmngr_free_block(physical_addr *address);
void pmmngr_free_blocks(physical_addr *address, uint32_t blocks);
uint32_t pmmngr_get_memory_size();
uint32_t pmmngr_get_block_count();
uint32_t pmmngr_get_used_block_count();
uint32_t pmmngr_get_free_block_count();
void pmmngr_init(physical_addr start_address, uint32_t size);

#endif // !PMM_H
