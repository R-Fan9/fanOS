#include "string.h"
#include "debug/display.h"
#include "stdint.h"

uint8_t *strchr(const uint8_t *str, const uint8_t c) {
  uint8_t *ptr = (uint8_t *)str;

  do {
    if (*ptr == c)
      return ptr;

    ptr++;
  } while (*ptr != '\0');

  return 0;
}

uint8_t *strcpy(uint8_t *dst, const uint8_t *src) {
  for (uint32_t i = 0; src[i]; i++) {
    dst[i] = src[i];
  }
  return dst;
}

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
