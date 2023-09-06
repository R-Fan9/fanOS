#include "malloc.h"
#include "C/stdbool.h"
#include "C/stdint.h"
#include "physical_mmngr.h"
#include "virtual_mmngr.h"

static malloc_block_t *malloc_list_head = 0;
static physical_addr malloc_virt_addr = 0;
static virtual_addr malloc_phys_addr = 0;
static uint32_t total_malloc_pages = 0;

// split a block by inserting a new block with the size of the differeence of
// original block and requested size
void malloc_split(malloc_block_t *node, const uint32_t size);

bool is_malloc_inited() { return malloc_list_head ? true : false; }

void *malloc_next_block(uint32_t size) {
  malloc_block_t *cur = 0;

  if (!size) {
    return 0;
  }

  if (!malloc_list_head->size) {
    malloc_init(size);
  }

  cur = malloc_list_head;
  while (cur->next && (cur->size < size || cur->free == false)) {
    cur = cur->next;
  }

  // current node size equals to the requested size
  if (cur->size == size) {
    cur->free = false;
  }
  // current node size is greater than the requested size
  else if (cur->size > size) {
    malloc_split(cur, size);
  }
  // current node size is less than the requested size
  // allocate more pages
  else {
    uint32_t page_count = 1;
    while (cur->size + page_count * PAGE_SIZE < size + sizeof(malloc_block_t)) {
      page_count++;
    }

    virtual_addr virt = malloc_virt_addr + total_malloc_pages * PAGE_SIZE;

    for (uint32_t i = 0; i < page_count; i++, virt += PAGE_SIZE) {
      pt_entry page = 0;
      physical_addr *phys = vmmngr_alloc_page(&page);
      if (phys) {
        vmmngr_map_page(phys, (virtual_addr *)virt);
        pt_entry_add_attrib(&page, PTE_WRITABLE);
        cur->size += PAGE_SIZE;
        total_malloc_pages++;
      }
    }

    malloc_split(cur, size);
  }

  return (void *)cur + sizeof(malloc_block_t);
}

// merge consecutive free blocks
void merge_free_blocks() {

  malloc_block_t *cur = malloc_list_head;

  while (cur && cur->next) {
    if (cur->free && cur->next) {
      cur->size += cur->next->size + sizeof(malloc_block_t);
      cur->next = cur->next->next;
    }
    cur = cur->next;
  }
}

void malloc_free(void *ptr) {
  for (malloc_block_t *cur = malloc_list_head; cur->next; cur = cur->next) {
    if ((void *)cur + sizeof(malloc_block_t) == ptr) {
      cur->free = true;
      merge_free_blocks();
      break;
    }
  }
}

void malloc_init(uint32_t bytes) {
  total_malloc_pages = bytes / PAGE_SIZE;
  if (bytes % PAGE_SIZE > 0) {
    total_malloc_pages++;
  }

  malloc_phys_addr = (physical_addr)pmmngr_alloc_blocks(total_malloc_pages);
  malloc_list_head = (malloc_block_t *)malloc_virt_addr;

  for (uint32_t i = 0, phys = malloc_phys_addr, virt = malloc_virt_addr;
       i < total_malloc_pages; i++, virt += PAGE_SIZE, phys += PAGE_SIZE) {

    vmmngr_map_page((physical_addr *)phys, (virtual_addr *)virt);

    pt_entry *page = vmmngr_get_page(virt);

    pt_entry_add_attrib(page, PTE_WRITABLE);
  }

  if (malloc_list_head) {
    malloc_list_head->size =
        total_malloc_pages * PAGE_SIZE - sizeof(malloc_block_t);
    malloc_list_head->free = true;
    malloc_list_head->next = 0;
  }
}

void malloc_split(malloc_block_t *node, const uint32_t size) {
  malloc_block_t *new_node =
      (malloc_block_t *)((void *)node + size + sizeof(malloc_block_t));

  new_node->size = node->size - size - sizeof(malloc_block_t);
  new_node->free = true;
  new_node->next = node->next;

  node->size = size;
  node->free = false;
  node->next = new_node;
}
