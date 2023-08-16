#include "vmmngr_pde.h"
#include "phyical_mmgnr.h"
#include "C/stdint.h"

void pd_entry_add_attrib(pd_entry *e, uint32_t attrib) { *e |= attrib; }

void pd_entry_del_attrib(pd_entry *e, uint32_t attrib) { *e &= ~(attrib); }

void pd_entry_set_frame(pd_entry *e, physical_addr addr) {
  *e = (*e & ~PDE_FRAME) | addr;
}

uint8_t pd_entry_is_present(pd_entry e) { return e & PDE_PRESENT; }

uint8_t pd_entry_is_user(pd_entry e) { return e & PDE_USER; }

uint8_t pd_entry_is_4mb(pd_entry e) { return e & PDE_4MB; }

uint8_t pd_entry_is_writable(pd_entry e) { return e & PDE_WRITABLE; }

uint32_t pd_entry_pfn(pd_entry e) { return e & PDE_FRAME; }
