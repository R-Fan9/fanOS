#include "C/stdbool.h"
#include "C/stdint.h"

#ifndef MALLOC_H
#define MALLOC_H

// linked list nodes for blocks of memory
typedef struct malloc_block {

  uint32_t size;             // size of this memory blocks in bytes
  bool free;                 // is this block of memory free
  struct malloc_block *next; // next block of memory

} malloc_block_t;

void *malloc_next_block(uint32_t bytes);
void merge_free_blocks();
bool is_malloc_inited();
void malloc_init(uint32_t bytes);
void malloc_free(void *ptr);

#endif // !MALLOC_H
