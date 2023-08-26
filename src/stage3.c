#include "C/ctype.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/exceptions.h"
#include "interrupts/floppydisk.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"

#define PREKERNEL_SIZE_ADDRESS 0x8000;
#define SMAP_ENTRY_COUNT_ADDRESS 0x1000;
#define SMAP_ENTRY_ADDRESS 0x1004;

typedef struct SMAP_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t acpi;
} __attribute__((packed)) SMAP_entry_t;

void hal_init();
void setup_interrupts();
void mmngr_init();

uint8_t *find_image(uint8_t *img_name, uint8_t *buffer) {
  for (uint32_t i = 0; i < bpbRootEntries; i++) {
    if (strncmp(img_name, &buffer[i * 32], 11) == 0) {
      return &buffer[i * 32];
    }
  }
  return 0;
}

void load_FAT(uint8_t *buffer) {
  uint32_t fat_sector = bpbReservedSectors;
  uint32_t fat_size = bpbNumberOfFATs * bpbSectorsPerFAT;
  for (uint32_t i = 0; i < fat_size; i++) {
    uint8_t *sector = fd_read_sector(fat_sector + i);
    if (sector) {
      memcpy(&buffer[i * bpbBytesPerSector], sector, bpbBytesPerSector);
    }
  }
}

void load_root(uint8_t *buffer) {
  uint32_t root_sector =
      bpbNumberOfFATs * bpbSectorsPerFAT + bpbReservedSectors;
  uint32_t root_size = 32 * bpbRootEntries / bpbBytesPerSector;

  for (uint32_t i = 0; i < root_size; i++) {
    uint8_t *sector = fd_read_sector(root_sector + i);
    if (sector) {
      memcpy(&buffer[i * bpbBytesPerSector], sector, bpbBytesPerSector);
    }
  }
}

__attribute__((section("prekernel_setup"))) void pkmain(void) {
  clear_screen();

  // initialize hardware abstraction layer (GDT, IDT, PIC)
  hal_init();
  setup_interrupts();

  // TODO - load kernel into physical address 0x100000
  fd_init(0);
  uint8_t *buffer = (uint8_t *)0x11000;
  load_root(buffer);
  uint8_t *img_addr = find_image((uint8_t *)"KRNL    SYS", buffer);
  if (img_addr) {
    uint16_t img_cluster = (uint16_t) * (uint8_t *)(img_addr + 26);
    load_FAT(buffer);
  }

  // initialize physical & virtual memory manager
  // mmngr_init();

  // TODO - once kernel is load to physical address 0x100000,
  // ((void (*)(void))0xC0000000)() to execute higher half kernel

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}

void hal_init() {

  // set up Global Descritor Table
  gdt_init();

  // set up Interrupt Descritor Table
  idt_init();

  // mask off all hardware interrupts, disable PIC
  disable_pic();

  // remap PIC IRQ interrupts (IRQ0 starts at 0x20)
  pic_init();
}

void setup_interrupts() {

  // set up exception handlers (i.e, divide by 0, page fault..)
  idt_set_descriptor(14, page_fault_handler, TRAP_GATE_FLAGS);

  // add ISRs for PIC hardware interrupts
  idt_set_descriptor(0x20, timer_irq0_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x21, keyboard_irq1_handler, INT_GATE_FLAGS);
  idt_set_descriptor(0x26, fd_irq6_handler, INT_GATE_FLAGS);

  // enable PIC IRQ interrupts after setting up their descriptors
  clear_irq_mask(0); // enable timer - IRQ0
  clear_irq_mask(1); // enable keyboard - IRQ1
  clear_irq_mask(6); // enable floppy disk - IRQ6

  // set default PIT Timer IRQ0 rate - ~1000hz
  // 1193182 MHZ / 1193 = ~1000
  pit_set_channel_mode_frequency(0, 1000, PIT_OCW_MODE_RATEGEN);

  // enable interrupts
  __asm__ __volatile__("sti");
}

void mmngr_init() {

  uint32_t prekernerl_size = *(uint32_t *)PREKERNEL_SIZE_ADDRESS;
  print_string((uint8_t *)"prekernel size: ");
  print_dec(prekernerl_size);
  print_string((uint8_t *)" sectors, ");
  print_dec(prekernerl_size * 512);
  print_string((uint8_t *)" bytes\n\n");

  uint32_t entry_count = *(uint32_t *)SMAP_ENTRY_COUNT_ADDRESS;
  SMAP_entry_t *entry = (SMAP_entry_t *)SMAP_ENTRY_ADDRESS;
  SMAP_entry_t *last_entry = entry + entry_count - 1;
  uint32_t total_memory = last_entry->base + last_entry->size - 1;
  print_string((uint8_t *)"Total memory: ");
  print_hex(total_memory);
  print_char('\n');

  // initialize physical memory manager
  pmmngr_init(0x30000, total_memory);

  for (uint32_t i = 0; i < entry_count; i++, entry++) {
    print_string((uint8_t *)"region: ");
    print_dec(i);
    print_string((uint8_t *)" start: ");
    print_hex(entry->base);
    print_string((uint8_t *)" size: ");
    print_hex(entry->size);
    print_string((uint8_t *)" type: ");
    print_dec(entry->type);
    print_char('\n');

    // entry with type 1 indicates the memory region is available
    if (entry->type == 1) {
      pmmngr_init_region(entry->base, entry->size);
    }
  }

  // deinitialize memory region below 0x12000 for BIOS & Bootloader
  pmmngr_deinit_region(0x1000, 0x11000);

  // deinitialize memory region where the prekernel is in
  pmmngr_deinit_region(0x50000, prekernerl_size * 512);

  // TODO - deinitialize memory region where the kernel is in
  //  pmmngr_deinit_region(0x100000, kernerl_size * 512);

  // deinitialize memory region where the memory map is in
  pmmngr_deinit_region(0x30000, pmmngr_get_block_count() / BLOCKS_PER_BYTE);

  print_string((uint8_t *)"\npmm total allocation blocks: ");
  print_dec(pmmngr_get_block_count());
  print_string((uint8_t *)"\npmm used blocks: ");
  print_dec(pmmngr_get_used_block_count());
  print_string((uint8_t *)"\npmm free blocks: ");
  print_dec(pmmngr_get_free_block_count());
  print_string((uint8_t *)"\n\n");

  // initialize virtual memory manager & enable paging
  vmmngr_init();
}
