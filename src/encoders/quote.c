#include <stdlib.h>
#include <string.h>
#include "encoders.h"

struct KeyValueMsg quote_encoder(const char* str, size_t strlength) {
  char* buffer = NULL;
  size_t size = sizeof(char) * (strlength);
  struct KeyValueMsg result = default_kvm;
  buffer = (char*)malloc(size);
  memcpy(buffer, str, size);
  if (buffer != NULL) {
    buffer[size - 1] = '\0';  // null terminated string (replacing \n)
    for (size_t i = 0; i < size; ++i) {
      if (buffer[i] == '\'') buffer[i] = '\"';
    }
    result.value = buffer;
    result.value_size = size;
  }
  
  return result;
}