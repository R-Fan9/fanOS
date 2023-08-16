#include "C/stdint.h"
#include "C/string.h"
#include "phyical_mmgnr.h"
#include "virtual_mmgnr.h"
#include "vmmngr_pde.h"
#include "vmmngr_pte.h"

static pdirectory *curdir = 0;

pt_entry *vmmngr_ptable_get_entry(ptable *table, virtual_addr addr);
pd_entry *vmmngr_pdirectory_get_entry(pdirectory *dir, virtual_addr addr);
pdirectory *vmmngr_get_directory();
void vmmngr_enable_paging();

uint8_t vmmngr_alloc_page(pt_entry *e) {

  // allocate a free physical frame
  void *p = pmmngr_alloc_block();
  if (!p) {
    return 0;
  }

  // map it to the page
  pt_entry_set_frame(e, (physical_addr)p);
  pt_entry_add_attrib(e, PTE_PRESENT);
  return 1;
}

void vmmngr_free_page(pt_entry *e) {

  void *p = (void *)pt_entry_pfn(*e);
  if (p) {
    pmmngr_free_block(p);
  }

  pt_entry_del_attrib(e, PTE_PRESENT);
}

void vmmngr_map_page(physical_addr *phys, virtual_addr *virt) {

  // get page directory
  pdirectory *dir = vmmngr_get_directory();

  // get page table
  pd_entry *e = &dir->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

  if ((*e & PTE_PRESENT) != PTE_PRESENT) {

    // page table not present, allocate it
    ptable *table = (ptable *)pmmngr_alloc_block();
    if (!table) {
      return;
    }

    // clear page table
    memset(table, 0, sizeof(ptable));

    //! create a new entry
    pd_entry *entry = &dir->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

    //! map in the table (Can also just do *entry |= 3) to enable these bits
    pd_entry_add_attrib(entry, PDE_PRESENT);
    pd_entry_add_attrib(entry, PDE_WRITABLE);
    pd_entry_set_frame(entry, (physical_addr)table);
  }

  //! get table
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);

  //! get page
  pt_entry *page = &table->entries[PAGE_TABLE_INDEX((uint32_t)virt)];

  //! map it in (Can also do (*page |= 3 to enable..)
  pt_entry_set_frame(page, (physical_addr)phys);
  pt_entry_add_attrib(page, PTE_PRESENT);
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

void vmmngr_flush_tlb_entry(virtual_addr addr) {
  __asm__ __volatile__("cli; invlpg (%0); sti" : : "r"(addr));
}

void vmmngr_initialize() {

  // allocate default page table
  ptable *table = (ptable *)pmmngr_alloc_block();
  if (!table) {
    return;
  }

  // allocates 3gb page table
  ptable *table2 = (ptable *)pmmngr_alloc_block();
  if (!table2) {
    return;
  }

  // clear page table
  memset(table, 0, sizeof(ptable));

  // 1st 4mb are idenitity mapped
  for (int i = 0, frame = 0x0, virt = 0x00000000; i < 1024;
       i++, frame += 4096, virt += 4096) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    table2->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // map 1MB physical address to 3GB virtual address
  // kernel is located here
  for (int i = 0, frame = 0x100000, virt = 0xc0000000; i < 1024;
       i++, frame += 4096, virt += 4096) {

    // create a new page
    pt_entry page = 0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_set_frame(&page, frame);

    // add it to the page table
    table->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // create default directory table
  pdirectory *dir = (pdirectory *)pmmngr_alloc_blocks(3);
  if (!dir) {
    return;
  }

  //! clear directory table and set it as current
  memset(dir, 0, sizeof(pdirectory));

  //! get first entry in dir table and set it up to point to our table
  pd_entry *entry = &dir->entries[PAGE_DIR_INDEX(0xc0000000)];
  pd_entry_add_attrib(entry, PDE_PRESENT);
  pd_entry_add_attrib(entry, PDE_WRITABLE);
  pd_entry_set_frame(entry, (physical_addr)table);

  pd_entry *entry2 = &dir->entries[PAGE_DIR_INDEX(0x00000000)];
  pd_entry_add_attrib(entry2, PDE_PRESENT);
  pd_entry_add_attrib(entry2, PDE_WRITABLE);
  pd_entry_set_frame(entry2, (physical_addr)table2);

  // switch to our page directory
  vmmngr_switch_pdirectory(dir);

  // enable paging
  vmmngr_enable_paging();
}

void vmmngr_enable_page() {
  // set PG (paging) bit 31 and PE (protection enable) bit 0 of CR0
  __asm__ __volatile__(
      "movl %CR0, %EAX; orl $0x80000001, %EAX; movl %EAX, %CR0");
}

pt_entry *vmmngr_ptable_get_entry(ptable *table, virtual_addr addr) {
  if (table)
    return &table->entries[PAGE_TABLE_INDEX(addr)];
  return 0;
}

pd_entry *vmmngr_pdirectory_get_entry(pdirectory *dir, virtual_addr addr) {
  if (dir)
    return &dir->entries[PAGE_TABLE_INDEX(addr)];
  return 0;
}

pdirectory *vmmngr_get_directory() { return curdir; }
