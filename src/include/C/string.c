#include "string.h"
#include "stdint.h"

int32_t strncmp(const uint8_t *str1, const uint8_t *str2, uint8_t len) {
  while (len > 0 && *str1 == *str2) {
    str1++;
    str2++;
    len--;
  }
  if (len == 0)
    return 0;
  else
    return *str1 - *str2;
}

void *memset(void *buffer, const uint8_t byte, const uint32_t len) {
  uint8_t *ptr = (uint8_t *)buffer;
  for (uint32_t i = 0; i < len; i++) {
    ptr[i] = byte;
  }
  return buffer;
}

void *memcpy(void *dst, const void *src, const uint32_t count) {
  for (uint32_t i = 0; i < count; i++) {
    ((uint8_t *)dst)[i] = ((uint8_t *)src)[i];
  }
  return dst;
}
