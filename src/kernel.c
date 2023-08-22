#include "C/stdint.h"
#include "debug/display.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"

physical_addr virt_to_phys(virtual_addr addr);

__attribute__((section("kernel_main"))) void kmain(void) {
  // clear_screen();

  // test paging
  uint32_t *ptr = (uint32_t *)0xC0000000;
  physical_addr addr = virt_to_phys((virtual_addr)ptr);
  if (addr) {
    print_hex((uint32_t)addr);
    print_char('\n');
    // print_hex((uint32_t)*(uint32_t *)addr);
    // print_char('\n');
  }
  print_hex(*ptr);

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}

physical_addr virt_to_phys(virtual_addr addr) {
  pdirectory *dir = vmmngr_get_directory();
  uint32_t pd_idx = (addr >> 22) & 0x3FF;
  print_string((uint8_t *)"page directory index: ");
  print_dec(pd_idx);
  print_char('\n');
  pd_entry *entry = &dir->entries[pd_idx];

  if (!pd_entry_is_present(*entry)) {
    print_string((uint8_t *)"NOT FOUND\npage directory entry value: ");
    print_hex((uint32_t)*entry);
    return 0;
  }

  ptable *table = (ptable *)(*entry & ~0xFFF);
  uint32_t pt_idx = (addr >> 12) & 0x3FF;
  print_string((uint8_t *)"page table index: ");
  print_dec(pt_idx);
  print_char('\n');
  pt_entry *page = &table->entries[pt_idx];

  if (!pt_entry_is_present(*page)) {
    print_string((uint8_t *)"NOT FOUND\npage table entry value: ");
    print_hex((uint32_t)*page);
    return 0;
  }

  uint32_t frame = *page & ~0xFFF;
  uint32_t offset = addr & 0xFFF;

  return (physical_addr)(frame + offset);
}
