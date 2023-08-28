#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "physical_mmngr.h"
#include "virtual_mmngr.h"
#include "vmmngr_pde.h"
#include "vmmngr_pte.h"

static pdirectory *curdir = 0;

pt_entry *vmmngr_ptable_get_entry(ptable *table, virtual_addr addr);
pd_entry *vmmngr_pdirectory_get_entry(pdirectory *dir, virtual_addr addr);
pt_entry *vmmngr_get_page(virtual_addr addr);
pd_entry *vmmngr_get_table(virtual_addr addr);

uint8_t vmmngr_alloc_page(pt_entry *e) {

  physical_addr *p = pmmngr_alloc_block();
  if (!p) {
    return 0;
  }

  pt_entry_set_frame(e, (physical_addr)p);
  pt_entry_add_attrib(e, PTE_PRESENT);
  return 1;
}

void vmmngr_free_page(pt_entry *e) {

  physical_addr *p = (physical_addr *)pt_entry_pfn(*e);
  if (p) {
    pmmngr_free_block(p);
  }

  pt_entry_del_attrib(e, PTE_PRESENT);
}

uint8_t vmmngr_switch_pdirectory(pdirectory *dir) {
  if (!dir) {
    return 0;
  }

  curdir = dir;
  physical_addr curpdbr = (physical_addr)&curdir->entries;

  // CR3 (Control register 3) holds address of the current page directory
  __asm__ __volatile__("movl %%EAX, %%CR3" : : "a"(curpdbr));
  return 1;
}

void vmmngr_map_page(physical_addr *phys_addr, virtual_addr *virt_addr) {

  // get page directory
  pdirectory *dir = vmmngr_get_directory();

  // get page table
  pd_entry *e = vmmngr_pdirectory_get_entry(dir, (virtual_addr)virt_addr);

  if (!pd_entry_is_present(*e)) {

    // page table not present, allocate it
    ptable *table = (ptable *)pmmngr_alloc_block();
    if (!table) {
      return;
    }

    // clear page table
    memset(table, 0, sizeof(ptable));

    // create a new entry
    pd_entry *entry = vmmngr_pdirectory_get_entry(dir, (virtual_addr)virt_addr);

    pd_entry_add_attrib(entry, PDE_PRESENT);
    pd_entry_add_attrib(entry, PDE_WRITABLE);
    pd_entry_set_frame(entry, (physical_addr)table);
  }

  // get table
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);

  // get page
  pt_entry *page = vmmngr_ptable_get_entry(table, (virtual_addr)virt_addr);
  pt_entry_add_attrib(page, PTE_PRESENT);
  pt_entry_set_frame(page, (physical_addr)phys_addr);
}

void vmmngr_unmap_page(virtual_addr *addr) {
  pt_entry *page = vmmngr_get_page((virtual_addr)addr);
  if (!page) {
    return;
  }
  pt_entry_set_frame(page, 0);
  pt_entry_del_attrib(page, PTE_PRESENT);
}

void vmmngr_init() {

  // allocate default page table
  ptable *default_page_table = (ptable *)pmmngr_alloc_block();
  if (!default_page_table) {
    return;
  }

  // allocates page table for virtual address starting at 3GB (for the kernel)
  ptable *kernel_page_table = (ptable *)pmmngr_alloc_block();
  if (!kernel_page_table) {
    return;
  }

  // clear page table
  memset(default_page_table, 0, sizeof(ptable));
  memset(kernel_page_table, 0, sizeof(ptable));

  // 1st 4MB are idenitity mapped
  for (uint32_t i = 0, frame = 0x0, virt = 0x00000000; i < PAGES_PER_TABLE;
       i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_add_attrib(&page, PTE_WRITABLE);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    default_page_table->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // map physical address at 1MB to virtual address at 3GB (for the kernel)
  for (uint32_t i = 0, frame = 0x100000, virt = 0xC0000000; i < PAGES_PER_TABLE;
       i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    kernel_page_table->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // create default directory table
  pdirectory *dir = (pdirectory *)pmmngr_alloc_blocks(3);
  if (!dir) {
    return;
  }

  // clear directory table and set it as current
  memset(dir, 0, sizeof(pdirectory));

  pd_entry *default_entry = vmmngr_pdirectory_get_entry(dir, 0x00000000);
  pd_entry_add_attrib(default_entry, PDE_PRESENT);
  pd_entry_add_attrib(default_entry, PDE_WRITABLE);
  pd_entry_set_frame(default_entry, (physical_addr)default_page_table);

  pd_entry *kernel_entry = vmmngr_pdirectory_get_entry(dir, 0xC0000000);
  pd_entry_add_attrib(kernel_entry, PDE_PRESENT);
  pd_entry_add_attrib(kernel_entry, PDE_WRITABLE);
  pd_entry_set_frame(kernel_entry, (physical_addr)kernel_page_table);

  // switch to our page directory
  vmmngr_switch_pdirectory(dir);

  // enable paging
  vmmngr_enable_paging();
}

void vmmngr_enable_paging() {
  // set PG (paging) bit 31 of CR0
  __asm__ __volatile__(
      "movl %CR0, %EAX; orl $0x80000000, %EAX; movl %EAX, %CR0");
}

void vmmngr_flush_tlb_entry(virtual_addr addr) {
  __asm__ __volatile__("cli; invlpg (%0); sti" : : "r"(addr));
}

pt_entry *vmmngr_ptable_get_entry(ptable *table, virtual_addr addr) {
  if (table)
    return &table->entries[PAGE_TABLE_INDEX(addr)];
  return 0;
}

pd_entry *vmmngr_pdirectory_get_entry(pdirectory *dir, virtual_addr addr) {
  if (dir)
    return &dir->entries[PAGE_DIR_INDEX(addr)];
  return 0;
}

pt_entry *vmmngr_get_page(virtual_addr addr) {
  pdirectory *dir = vmmngr_get_directory();
  pd_entry *e = vmmngr_pdirectory_get_entry(dir, addr);
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);
  pt_entry *page = vmmngr_ptable_get_entry(table, addr);
  return page;
}

pd_entry *vmmngr_get_table(virtual_addr addr) {
  pdirectory *dir = vmmngr_get_directory();
  pd_entry *table = vmmngr_pdirectory_get_entry(dir, addr);
  return table;
}

pdirectory *vmmngr_get_directory() { return curdir; }

physical_addr *vmmngr_virt_to_phys(virtual_addr *addr) {
  pt_entry *page = vmmngr_get_page((virtual_addr)addr);
  physical_addr *frame = (physical_addr *)PAGE_GET_PHYSICAL_ADDRESS(page);
  uint32_t offset = (uint32_t)addr & 0xFFF;
  return frame + offset;
}
