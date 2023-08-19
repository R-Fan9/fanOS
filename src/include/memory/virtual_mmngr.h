#include "C/stdint.h"
#include "vmmngr_pde.h"
#include "vmmngr_pte.h"

#ifndef VMM_H
#define VMM_H

typedef uint32_t virtual_addr;

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024

// page table represents 4MB address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// directory table represents 4GB address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// page sizes are 4k
#define PAGE_SIZE 4096

// the most significant 10 bits (bit 22-31) specify the index of the page
// directory entry 0x3FF = 1023, guarantees the index is within 1024
#define PAGE_DIR_INDEX(x) ((x >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x) ((x >> 12) & 0x3FF)

// physical address is at the higher 20 bits (bits 12-31) of a page table entry
// 0xFFF = 000000000000000000111111111111
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xFFF)

typedef struct {
  pt_entry entries[PAGES_PER_TABLE];
} ptable;

typedef struct {
  pd_entry entries[TABLES_PER_DIR];
} pdirectory;

// allocate a page in physical memory
uint8_t vmmngr_alloc_page(pt_entry *e);

// free a page in physical memory
void vmmngr_free_page(pt_entry *e);

// switch to a new page directory
uint8_t vmmngr_switch_pdirectory(pdirectory *dir);

// flush a TLB entry
void vmmngr_flush_tlb_entry(virtual_addr addr);

// map physical address to vitrual address
void vmmngr_map_page(physical_addr *phys_addr, virtual_addr *virt_addr);

// unmap physical address from vitrual address
void vmmngr_unmap_page(virtual_addr *addr);

// enable paging
void vmmngr_enable_paging();

// initalize VMM
void vmmngr_init();

#endif // !VMM_H
