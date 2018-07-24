#include <stdlib.h>
#include <string.h>
#include "http_query.h"
#include "encoders.h"

/*  decode_hex return 0 if the hex number
    is illegal or falls in 0~0x1F. */
char escp_decode_hex(const char* pSrc) {
  char c1 = pSrc[0];
  if (0 == c1) return 0;
  char c2 = pSrc[1];
  if (0 == c2) return 0;

  int v1 = char2int(c1);
  int v2 = char2int(c2);
  if (v1 < 0 || v2 < 0) /* One of c1 and c2 is illegal character. */
    return 0;

  unsigned char result = 0;
  result = v1 * 16 + v2;

  /* if char encoding between %00 to %1F, relpace to space */
  if (result <= 0x1F) return ' ';

  return (char)result;
}

struct KeyValueMsg url_escape_encoder(const char* str, size_t strlength) {
  char* escapeResult = NULL;
  size_t size = sizeof(char) * (strlength);
  char* pResult = (char*)malloc(size);
  char* pCur = pResult;
  size_t escapeSize = 0;
  char* pch;
  struct KeyValueMsg result = default_kvm;

  if (size != 0) {
    for (; strlength; ++str, --strlength) {
      char cResult = 0;
      if (*str == '%') {
        /* Do HEX decode */
        if (strlength < 3) {
          break;
        }
        cResult = escp_decode_hex(str + 1);
        str += 2;
        strlength -= 2;
      } else {
        cResult = *str;
      }
      *pCur = cResult;
      ++pCur;
    }
  }

  pch = strchr(pResult, '\n');
  if (pch != NULL) {
    escapeSize = pch - pResult + 1;
  }

  if (escapeSize != 0) {
    escapeResult = (char*)malloc(escapeSize);
    memcpy(escapeResult, pResult, escapeSize);
    ((char*)escapeResult)[escapeSize - 1] =
        '\0'; /* null terminated string (replacing \n) */
  }
  free(pResult);
  result.value = escapeResult;
  result.value_size = escapeSize;
  return result;
}
