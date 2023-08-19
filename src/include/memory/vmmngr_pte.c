#include "vmmngr_pte.h"
#include "C/stdint.h"
#include "phyical_mmgnr.h"

void pt_entry_add_attrib(pt_entry *e, uint32_t attrib) { *e |= attrib; }

void pt_entry_del_attrib(pt_entry *e, uint32_t attrib) { *e &= ~(attrib); }

void pt_entry_set_frame(pt_entry *e, physical_addr addr) {
  *e = (*e & ~PTE_FRAME) | addr;
}

uint8_t pt_entry_is_present(pt_entry e) { return e & PTE_PRESENT; }

uint8_t pt_entry_is_writable(pt_entry e) { return e & PTE_WRITABLE; }

physical_addr pt_entry_pfn(pt_entry e) { return e & PTE_FRAME; }
