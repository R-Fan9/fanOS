#include "exceptions.h"
#include "C/stdint.h"
#include "debug/display.h"

__attribute__((interrupt)) void page_fault_handler(int_frame_t *frame,
                                                   uint32_t error_code) {
  (void)frame;

  print_string((uint8_t *)"PAGE FAULT EXCEPTION (#PF)\r\nERROR CODE: ");
  print_hex(error_code);

  uint32_t bad_addr = 0;

  // CR2 contains bad address that caused page fault
  __asm__ __volatile__("movl %%CR2, %0" : "=r"(bad_addr));

  print_string((uint8_t *)"\r\nADDRESS: ");
  print_hex(bad_addr);

  // TODO - Replace with actual implementations for mapping in page and setting
  // present, read/write, etc.
  __asm__ __volatile__("cli;hlt");
}
