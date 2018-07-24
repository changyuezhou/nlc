#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_query.h"

bool decode_single_http_query(HttpQuery& result, const char* pQueryString,
                              size_t queryLen) {
  // Find '=' in pQueryString to
  int idxOfEqual = -1;
  const char* pTmpPtr = pQueryString;
  for (size_t i = 0; i <= queryLen; ++i, ++pTmpPtr) {
    if (*pTmpPtr == '=') {
      idxOfEqual = i;
      break;
    }
  }

  if (idxOfEqual < 0) {
    return false;  // Mailform
  }

  // If the name part is empty, ignore
  // this field.
  if (idxOfEqual == 0) {
    return true;
  }

  char name[2048] = {};
  char value[2048] = {};
  size_t outputLen = sizeof(name) - 1;
  if (!decode_url(name, &outputLen, pQueryString, idxOfEqual)) {
    return false;
  }
  outputLen = sizeof(name) - 1;
  if (!decode_url(value, &outputLen, pQueryString + idxOfEqual + 1,
                  queryLen - idxOfEqual - 1)) {
    return false;
  }

  result[name] = value;
  return true;
}

bool decode_http_query(HttpQuery& result, const char* pQueryString) {
  result.clear();

  // Use '&' to seperate the whole query string.
  const char* pBegin = pQueryString;
  const char* pCur = pQueryString;
  for (; *pCur; ++pCur) {
    if (*pCur == '&') {
      if (pCur != pBegin) {
        if (!decode_single_http_query(result, pBegin, pCur - pBegin))
          return false;
      }
      pBegin = pCur + 1;
    }
  }

  if (pCur != pBegin) {
    if (!decode_single_http_query(result, pBegin, pCur - pBegin)) return false;
  }

  return true;
}

int char2int(char ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  else if (ch >= 'a' && ch <= 'f')
    return 10 + ch - 'a';
  else if (ch >= 'A' && ch <= 'F')
    return 10 + ch - 'A';

  return -1;
}

// decode_hex return 0 if the hex number
// is illegal or falls in 0~0x1F.
char decode_hex(const char* pSrc) {
  char c1 = pSrc[0];
  if (0 == c1) return 0;
  char c2 = pSrc[1];
  if (0 == c2) return 0;

  int v1 = char2int(c1);
  int v2 = char2int(c2);
  if (v1 < 0 || v2 < 0)  // One of c1 and c2 is illegal character.
    return 0;

  unsigned char result = 0;
  result = v1 * 16 + v2;

  if (result <= 0x1F) return 0;

  return (char)result;
}

bool decode_url(char* pResult, size_t* pSize, const char* pUrlString,
                size_t urlLen) {
  if (*pSize <= urlLen) {
    *pSize = urlLen + 1;
    return false;
  }

  char* pCur = pResult;
  const char* pSrc = pUrlString;
  for (; urlLen; ++pSrc, --urlLen) {
    unsigned char cResult = 0;
    if (*pSrc == '+') {
      cResult = ' ';
    } else if (*pSrc == '%') {
      // Do HEX decode.
      if (urlLen < 3) {
        // Stop decoding.
        break;
      }
      cResult = decode_hex(pSrc + 1);
      pSrc += 2;  // decode_hex will read 2 more character.
      urlLen -= 2;
    } else {
      cResult = *pSrc;
    }

    // Ignore the character is it's less or equals to 0x1F and not a new line.
    if (cResult <= 0x1F && (cResult != 0x0A || cResult != 0x0D)) continue;

    *pCur = cResult;
    ++pCur;
  }

  *pCur = 0;  // Add NULL terminator.
  return true;
}

//#define __UNIT_TEST__
#ifdef __UNIT_TEST__
int main(int argc, char** argv) {
  const char* testString =
      "Mozilla%2F5.0+%28Linux%3B+Android+4.4.2%3B+SGH-N075T+Build%2FKOT49H%29+"
      "AppleWebKit%2F537.36+%28KHTML%2C+like+Gecko%29+Version%2F4.0+Chrome%"
      "2F30.0.0.0+Mobile+Safari%2F537.36";
  size_t destLen = strlen(testString) + 1;
  char* pResult = (char*)malloc(destLen);

  bool result = decode_url(pResult, &destLen, testString, strlen(testString));
  if (!result)
    printf("Wrong input.\n");
  else
    printf("The result is:\n%s\n", pResult);

  free(pResult);

  /*
  const char* testQuery = "an=%E5%A4%A2%E8%B2%82%E8%9F%ACAPP";
  HttpQuery   httpResult;
  decode_single_http_query(httpResult, testQuery, strlen(testQuery));
  printf("%s\n", httpResult["an"].c_str());
  */
  {
    const char* testQuery =
        "&&&&&=abc&&&&v=1&t=install&itv=hasoffers&tm=1425102480&imei=&andid=&"
        "gaid=A2193FB8-7DA2-481B-BAEE-EA6C47C89BA4&idfa=&iosadtrac=1&ul="
        "Chinese&uc=TW&ucar=&uip=114.40.192.238&ua=Mozilla%2F5.0+%28Linux%3B+U%"
        "3B+Android+4.1.2%3B+zh-tw%3B+ST26i+Build%2F11.2.A.0.31%29+AppleWebKit%"
        "2F534.30+%28KHTML%2C+like+Gecko%29+Version%2F4.0+Mobile+Safari%2F534."
        "30&at=Android&apkgn=com.mofun.ftg2.taiwan&an=%E7%99%BC%E5%B0%84%E5%90%"
        "A7%21%E7%A1%AC%E6%BC%A2&av=4&cn=&pb=&ad=&clid=&cs=&spb=&imt=&isattrib="
        "0&isinstattrib=0&isassist=0&&&&&&\n";
    HttpQuery httpResult;
    if (!decode_http_query(httpResult, testQuery)) printf("Decode Fail!\n");

    HttpQuery::const_iterator iter = httpResult.begin();
    HttpQuery::const_iterator end = httpResult.end();
    for (; iter != end; ++iter) {
      printf("%s=%s\n", iter->first.c_str(), iter->second.c_str());
    }
  }

  return 0;
}
#endif
