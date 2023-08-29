#include "C/ctype.h"
#include "C/stdint.h"
#include "C/string.h"
#include "debug/display.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/pic.h"
#include "interrupts/exceptions.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"
#include "storage/fat12.h"
#include "storage/floppydisk.h"

#define SMAP_ENTRY_COUNT_ADDRESS 0x1000
#define SMAP_ENTRY_ADDRESS 0x1004
#define MEMMAP_ADDRESS 0x30000
#define PREKERNEL_ADDRESS 0x50000
#define PREKERNEL_SIZE_ADDRESS 0x8000
#define KERNEL_ADDRESS 0x100000
#define KERNEL_IMAGE "KRNL    SYS"

typedef struct SMAP_entry {
  uint64_t base;
  uint64_t size;
  uint32_t type;
  uint32_t acpi;
} __attribute__((packed)) SMAP_entry_t;

void hal_init();
void setup_interrupts();

__attribute__((section("prekernel_setup"))) void pkmain(void) {
  clear_screen();

  // initialize hardware abstraction layer (GDT, IDT, PIC)
  hal_init();
  setup_interrupts();

  uint32_t prekernel_size = *(uint32_t *)PREKERNEL_SIZE_ADDRESS;
  uint32_t entry_count = *(uint32_t *)SMAP_ENTRY_COUNT_ADDRESS;
  SMAP_entry_t *entry = (SMAP_entry_t *)SMAP_ENTRY_ADDRESS;
  SMAP_entry_t *last_entry = entry + entry_count - 1;
  uint32_t total_memory = last_entry->base + last_entry->size - 1;

  // initialize physical memory manager
  pmmngr_init(MEMMAP_ADDRESS, total_memory);

  for (uint32_t i = 0; i < entry_count; i++, entry++) {
    // entry with type 1 indicates the memory region is available
    if (entry->type == 1) {
      pmmngr_init_region(entry->base, entry->size);
    }
  }

  // initialize floppy disk controller
  fd_init(0);

  // load root directory table
  uint8_t *buffer = (uint8_t *)0x11000;
  fat_load_root(buffer);

  // find kernel image entry in the root directory table
  uint8_t *img_addr = fat_find_image((uint8_t *)KERNEL_IMAGE, buffer);
  if (img_addr) {

    // first cluster of every entry is at byte 26
    uint16_t img_cluster = (uint16_t) * (uint16_t *)(img_addr + 26);

    // load FAT table
    fat_load_FAT(buffer);

    // load kernel into physical address 0x100000
    uint8_t *kernel = (uint8_t *)KERNEL_ADDRESS;
    uint32_t kernel_size = fat_load_image(kernel, buffer, img_cluster);

    // deinitialize memory region where the kernel is in
    pmmngr_deinit_region((physical_addr)(uint32_t *)KERNEL_ADDRESS,
                         kernel_size * 512);

    // deinitialize memory region below 0x15000 for
    // BIOS, Bootloader & FDC
    pmmngr_deinit_region(0x1000, 0x14000);

    // deinitialize memory region where the prekernel is in
    pmmngr_deinit_region(PREKERNEL_ADDRESS, prekernel_size * 512);

    // deinitialize memory region where the memory map is in
    pmmngr_deinit_region(MEMMAP_ADDRESS,
                         pmmngr_get_block_count() / BLOCKS_PER_BYTE);

    // initialize virtual memory manager & enable paging
    vmmngr_init();

    // execute higher half kernel
    ((void (*)(void))0xC0000000)();
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
