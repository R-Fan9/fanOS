#include "C/stdint.h"
#include "phyical_mmgnr.h"

#ifndef VMMNGR_PDE
#define VMMNGR_PDE

enum PAGE_PDE_FLAGES {
  PDE_PRESENT = 1,        // 0000000000000000000000000000001
  PDE_WRITABLE = 2,       // 0000000000000000000000000000010
  PDE_USER = 4,           // 0000000000000000000000000000100
  PDE_PWT = 8,            // 0000000000000000000000000001000
  PDE_PCD = 0x10,         // 0000000000000000000000000010000
  PDE_ACCESSED = 0x20,    // 0000000000000000000000000100000
  PDE_DIRTY = 0x40,       // 0000000000000000000000001000000
  PDE_4MB = 0x80,         // 0000000000000000000000010000000
  PDE_CPU_GLOBAL = 0x100, // 0000000000000000000000100000000
  PDE_LV4_GLOBAL = 0x200, // 0000000000000000000001000000000
  PDE_FRAME = 0x7FFFF000  // 1111111111111111111000000000000	I86_PDE_PRESENT
};

typedef uint32_t pd_entry;

// sets a flag in the page table entry
void pd_entry_add_attrib(pd_entry *e, uint32_t attrib);

// clears a flag in the page table entry
void pd_entry_del_attrib(pd_entry *e, uint32_t attrib);

// sets a frame to page table entry
void pd_entry_set_frame(pd_entry *e, physical_addr addr);

// test if page is present
uint8_t pd_entry_is_present(pd_entry e);

// test if directory is user mode
uint8_t pd_entry_is_user(pd_entry e);

// test if directory contains 4mb pages
uint8_t pd_entry_is_4mb(pd_entry e);

// test if page is writable
uint8_t pd_entry_is_writable(pd_entry e);

// get page table entry frame address
uint32_t pd_entry_pfn(pd_entry e);

#endif // !VMMNGR_PDE
