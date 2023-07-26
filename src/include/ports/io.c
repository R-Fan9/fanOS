#include "io.h"

// write AL to port DX
void outb(uint16_t port, uint8_t value) {
  __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

// write AX to port DX
void outw(uint16_t port, uint16_t value) {
  __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

// read in value from port DX to AL and return AL
uint8_t inb(uint16_t port) {
  uint8_t ret_val;

  __asm__ __volatile__("intb %1, %0" : "=a"(ret_val) : "Nd"(port));

  return ret_val;
}

// read in value from port DX to AX and return AX
uint16_t inw(uint16_t port) {
  uint16_t ret_val;

  __asm__ __volatile__("intb %1, %0" : "=a"(ret_val) : "Nd"(port));

  return ret_val;
}
