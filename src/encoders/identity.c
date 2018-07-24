#include <stdlib.h>
#include <string.h>
#include "encoders.h"

struct KeyValueMsg identity_encoder(const char* str, size_t strlength) {
  void* buffer = NULL;
  size_t size = sizeof(char) * (strlength);
  struct KeyValueMsg result = default_kvm;

  buffer = malloc(size);
  memcpy(buffer, str, size);
  if (buffer != NULL) {
    ((char*)buffer)[strlength - 1] = '\0';  // null terminated string (replacing \n)
    result.value = buffer;
    result.value_size = size;
  }
  return result;
}
