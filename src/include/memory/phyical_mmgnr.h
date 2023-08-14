#include "C/stdint.h"

#ifndef PMM_H
#define PMM_H

// 8 blocks per byte
#define BLOCKS_PER_BYTE 8

// block size (4k)
#define BLOCK_SIZE 4096

// block alignment
#define BLOCK_ALIGN PMMNGR_BLOCK_SIZE

void pmmngr_init(uint32_t start_address, uint32_t size);
void pmmngr_init_region(uint32_t base, uint32_t size);
void pmmngr_deinit_region(uint32_t base, uint32_t size);
uint32_t *pmmngr_alloc_block();
uint32_t *pmmngr_alloc_blocks(uint32_t blocks);
void pmmngr_free_block(uint32_t *address);
void pmmngr_free_blocks(uint32_t *address, uint32_t blocks);
uint32_t pmmngr_get_memory_size();
uint32_t pmmngr_get_block_count();
uint32_t pmmngr_get_used_block_count();
uint32_t pmmngr_get_free_block_count();

#endif // !PMM_H
