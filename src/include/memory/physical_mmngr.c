#include "physical_mmngr.h"
#include "C/stdbool.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"

// size of physical memory
static uint32_t memory_size = 0;

// number of blocks currently used
static uint32_t used_blocks = 0;

// maximum number of available mmemory blocks
static uint32_t max_blocks = 0;

// memory map bit array, each bit represetns a memory block (1 - allocated, 0 -
// available)
static uint32_t *memory_map = 0;

void mmap_set(uint32_t bit);
void mmap_unset(uint32_t bit);
int32_t mmap_first_free_blocks(uint32_t blocks);

void mmap_set(uint32_t bit) { memory_map[bit / 32] |= (1 << (bit % 32)); }

void mmap_unset(uint32_t bit) { memory_map[bit / 32] &= ~(1 << (bit % 32)); }

int32_t mmap_first_free_blocks(uint32_t blocks) {
  if (blocks == 0) {
    return -1;
  }

  uint32_t mmap_size = pmmngr_get_block_count() / 32;

  for (uint32_t i = 0; i < mmap_size; i++) {
    if (memory_map[i] != 0xFFFFFFFF) {
      for (uint32_t j = 0; j < 32; j++) {

        if (!(memory_map[i] & (1 << (j % 32)))) {
          for (uint32_t count = 0, free_blocks = 0; count < blocks; count++) {
            uint32_t idx = i;
            uint32_t bit = j + count;
            uint32_t idx_count = bit / 32;
            for (; idx_count > 0 && idx + 1 < mmap_size; idx_count--) {
              idx++;
            }

            if (!(memory_map[idx] & (1 << (bit % 32)))) {
              free_blocks++;
            }

            if (free_blocks == blocks) {
              return i * 32 + j;
            }
          }
        }
      }
    }
  }

  return -1;
}

void pmmngr_init(physical_addr start_address, uint32_t size) {
  memory_map = (uint32_t *)start_address;
  memory_size = size;
  max_blocks = pmmngr_get_memory_size() / BLOCK_SIZE;
  used_blocks = pmmngr_get_block_count();

  // by default, set all memory in use (used blocks = 1)
  // each byte of memory map holds 8 blocks
  memset(memory_map, 0xFF, pmmngr_get_block_count() / BLOCKS_PER_BYTE);
}

// initialize region of memory (set blocks free/available)
void pmmngr_init_region(physical_addr base, uint32_t size) {
  int32_t align = base / BLOCK_SIZE;
  int32_t blocks = size / BLOCK_SIZE;
  for (; blocks > 0; blocks--) {
    mmap_unset(align++);
    used_blocks--;
  }

  // first block is always set, which insures allocs cant be 0
  mmap_set(0);
}

// deinitialize region of memory (set blocks used/reserved)
void pmmngr_deinit_region(physical_addr base, uint32_t size) {
  int32_t align = base / BLOCK_SIZE;
  int32_t blocks = size / BLOCK_SIZE;
  for (; blocks > 0; blocks--) {
    mmap_set(align++);
    used_blocks++;
  }
}

physical_addr *pmmngr_alloc_block() { return pmmngr_alloc_blocks(1); }

physical_addr *pmmngr_alloc_blocks(uint32_t blocks) {

  // not enough blocks to allocate
  if (pmmngr_get_free_block_count() <= blocks) {
    return 0;
  }
  int32_t starting_block = mmap_first_free_blocks(blocks);

  // out of memory
  if (starting_block == -1) {
    return 0;
  }

  for (uint32_t i = 0; i < blocks; i++) {
    mmap_set(starting_block + i);
  }

  used_blocks += blocks;

  // convert blocks to bytes to get start of actual RAM that is now allocated
  physical_addr address = starting_block * BLOCK_SIZE;
  return (physical_addr *)address;
}

void pmmngr_free_block(physical_addr *address) {
  pmmngr_free_blocks(address, 1);
}

void pmmngr_free_blocks(physical_addr *address, uint32_t blocks) {
  uint32_t starting_block = (physical_addr)address / BLOCK_SIZE;

  for (uint32_t i = 0; i < blocks; i++) {
    mmap_unset(starting_block + i);
  }

  used_blocks -= blocks;
}

void pmmngr_display_blocks() {
  print_string((uint8_t *)"\n\npmm total allocation blocks: ");
  print_dec(pmmngr_get_block_count());
  print_string((uint8_t *)"\npmm used blocks: ");
  print_dec(pmmngr_get_used_block_count());
  print_string((uint8_t *)"\npmm free blocks: ");
  print_dec(pmmngr_get_free_block_count());
  clear_screen();
}

uint32_t pmmngr_get_memory_size() { return memory_size; }
uint32_t pmmngr_get_block_count() { return max_blocks; }
uint32_t pmmngr_get_used_block_count() { return used_blocks; }
uint32_t pmmngr_get_free_block_count() { return max_blocks - used_blocks; }
