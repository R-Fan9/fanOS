#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "phyical_mmgnr.h"
#include "virtual_mmgnr.h"
#include "vmmngr_pde.h"
#include "vmmngr_pte.h"

static pdirectory *curdir = 0;

pt_entry *vmmngr_ptable_get_entry(ptable *table, virtual_addr addr);
pd_entry *vmmngr_pdirectory_get_entry(pdirectory *dir, virtual_addr addr);
pdirectory *vmmngr_get_directory();
pt_entry *vmmngr_get_page(virtual_addr addr);
void vmmngr_enable_paging();

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

  if ((*e & PDE_PRESENT) != PDE_PRESENT) {

    // page table not present, allocate it
    ptable *table = (ptable *)pmmngr_alloc_block();
    if (!table) {
      return;
    }

    // clear page table
    memset(table, 0, sizeof(ptable));

    // create a new entry
    pd_entry *entry = vmmngr_pdirectory_get_entry(dir, (virtual_addr)virt_addr);

    // map in the table (Can also just do *entry |= 3) to enable these bits
    pd_entry_add_attrib(entry, PDE_PRESENT);
    pd_entry_add_attrib(entry, PDE_WRITABLE);
    pd_entry_set_frame(entry, (physical_addr)table);
  }

  // get table
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);

  // get page
  pt_entry *page = vmmngr_ptable_get_entry(table, (virtual_addr)virt_addr);

  // map it in (Can also do (*page |= 3 to enable..)
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
  ptable *table = (ptable *)pmmngr_alloc_block();
  if (!table) {
    return;
  }

  // allocates page table for virtual address starting at 3GB (for the kernel)
  ptable *table2 = (ptable *)pmmngr_alloc_block();
  if (!table2) {
    return;
  }

  // clear page table
  memset(table, 0, sizeof(ptable));
  memset(table2, 0, sizeof(ptable));

  // 1st 4MB are idenitity mapped
  for (uint32_t i = 0, frame = 0x0, virt = 0x00000000; i < 1024;
       i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_add_attrib(&page, PTE_WRITABLE);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    table2->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // map 1MB physical address to 3GB virtual address
  // kernel is located here
  for (uint32_t i = 0, frame = 0x100000, virt = 0xC0000000; i < 1024;
       i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_add_attrib(&page, PTE_WRITABLE);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    table->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // create default directory table
  pdirectory *dir = (pdirectory *)pmmngr_alloc_blocks(3);
  if (!dir) {
    return;
  }

  // clear directory table and set it as current
  memset(dir, 0, sizeof(pdirectory));

  // get first entry in dir table and set it up to point to our table
  pd_entry *entry = vmmngr_pdirectory_get_entry(dir, 0xC0000000);
  pd_entry_add_attrib(entry, PDE_PRESENT);
  pd_entry_add_attrib(entry, PDE_WRITABLE);
  pd_entry_set_frame(entry, (physical_addr)table);

  pd_entry *entry2 = vmmngr_pdirectory_get_entry(dir, 0x00000000);
  pd_entry_add_attrib(entry2, PDE_PRESENT);
  pd_entry_add_attrib(entry2, PDE_WRITABLE);
  pd_entry_set_frame(entry2, (physical_addr)table2);

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

pdirectory *vmmngr_get_directory() { return curdir; }

pt_entry *vmmngr_get_page(virtual_addr addr) {
  pdirectory *dir = vmmngr_get_directory();
  pd_entry *e = vmmngr_pdirectory_get_entry(dir, addr);
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);
  pt_entry *page = vmmngr_ptable_get_entry(table, addr);
  return page;
}
