#include "string.h"

void *memset(void *buffer, uint8_t byte, uint32_t len) {
  uint8_t *ptr = (uint8_t *)buffer;

  for (uint32_t i = 0; i < len; i++) {
    ptr[i] = byte;
  }
  return buffer;
}
