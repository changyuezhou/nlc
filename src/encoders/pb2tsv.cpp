#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <utility>
#include "http_query.h"
extern "C" {
#include "encoders.h"
}

typedef std::pair<const char*, const char*> StrRange;
typedef std::vector<StrRange> StrRanges;

void breakString(StrRanges& ranges, const char* pSrc, char cToken) {
  ranges.clear();
  const char* pBegin = pSrc;
  const char* pCur = pSrc;

  for (; *pCur != 0; ++pCur) {
    if (*pCur == cToken) {
      ranges.push_back(std::make_pair(pBegin, pCur));
      pBegin = pCur + 1;
    }
  }

  ranges.push_back(std::make_pair(pBegin, pCur));
}

void pushTSVValue(std::vector<std::string>& tsvValues,
                  const std::string& value) {
  if (value.empty() || value == "-")
    tsvValues.push_back("\\N");
  else
    tsvValues.push_back(value);
}

extern "C"  // This line prevent C++ compiler to do name mangling for pb2tsv
            // function.
    struct KeyValueMsg
    pb2tsv(const char* pLine, size_t n) {
  const char* pTimeStamp = pLine;
  const char* pHttpRequest = strchr(pLine, ' ');
  struct KeyValueMsg result = default_kvm;
  if (pHttpRequest == 0) return result;
  ++pHttpRequest;

  std::string timeStamp(pTimeStamp, pHttpRequest - 1 - pTimeStamp);
  HttpQuery httpQuery;
  if (!decode_http_query(httpQuery, pHttpRequest)) return result;

  // Find itv
  std::string itv = httpQuery["itv"];

  std::vector<std::string> tsvValues;
  // Generate TSV
  // The order of values of TSV is crucial to HIVE, don't
  // change it!!
  // 1. create_at
  pushTSVValue(tsvValues, timeStamp);
  // 2. action
  pushTSVValue(tsvValues, httpQuery["t"]);
  // 3. vendor
  pushTSVValue(tsvValues, httpQuery["itv"]);
  // 4. app_pkg_name
  pushTSVValue(tsvValues, httpQuery["apkgn"]);
  // 5. app_pkg_ver
  pushTSVValue(tsvValues, httpQuery["av"]);
  // 6. ip
  pushTSVValue(tsvValues, httpQuery["uip"]);
  // 7. gaid
  pushTSVValue(tsvValues, httpQuery["gaid"]);
  // 8. idfa
  pushTSVValue(tsvValues, httpQuery["idfa"]);
  // 9. ad_id
  pushTSVValue(tsvValues, httpQuery["ad"]);
  // 10. license_key
  pushTSVValue(tsvValues, httpQuery["pb"]);
  // 11. language
  pushTSVValue(tsvValues, httpQuery["ul"]);
  // 12. country
  pushTSVValue(tsvValues, httpQuery["uc"]);
  // 13. user_agent
  pushTSVValue(tsvValues, httpQuery["ua"]);
  // 14. isinstattrib
  pushTSVValue(tsvValues, httpQuery["isinstattrib"]);
  // 15. isassist
  pushTSVValue(tsvValues, httpQuery["isassist"]);
  // 16. campaign
  pushTSVValue(tsvValues, httpQuery["cn"]);
  // 17. click id
  pushTSVValue(tsvValues, httpQuery["clid"]);

  size_t resultSize = tsvValues.size() - 1;
  for (size_t i = 0; i < tsvValues.size(); ++i) {
    resultSize += tsvValues.at(i).length();
  }
  resultSize += 1;  // one byte for NULL terminator.

  char* pResult = (char*)malloc(resultSize);
  pResult[resultSize - 1] = 0;
  char* pDstPtr = pResult;
  for (size_t i = 0; i < tsvValues.size(); ++i) {
    if (i != 0) {
      *pDstPtr = '\t';
      ++pDstPtr;
    }
    const char* pSrc = tsvValues.at(i).c_str();
    size_t srcLength = tsvValues.at(i).length();
    memcpy(pDstPtr, pSrc, srcLength);
    pDstPtr += srcLength;
  }

  result.value = pResult;
  result.value_size = resultSize;
  return result;
}

//#define __UNIT_TEST__
#ifdef __UNIT_TEST__
int main(int argc, char** argv) {
  const char* testString =
      "1416276917 "
      "v=1&t=install&itv=hasoffers&tm=1416276917&imei=&andid=&gaid=6662A0A7-"
      "3142-4757-BEC8-5526D584347F&idfa=&iosadtrac=1&ul=Chinese&uc=TW&ucar=%EF%"
      "BF%BDr%5E%EF%BF%BD&uip=39.8.144.244&ua=Mozilla%2F5.0+%28Linux%3B+U%3B+"
      "Android+4.3%3B+zh-tw%3B+GT-N7100+Build%2FJSS15J%29+AppleWebKit%2F534.30+"
      "%28KHTML%2C+like+Gecko%29+Version%2F4.0+Mobile+Safari%2F534.30&at="
      "Android&apkgn=com.mofun.ftg2.taiwan&an=%E7%99%BC%E5%B0%84%E5%90%A7%21%"
      "E7%A1%AC%E6%BC%A2&av=2&cn=&pb=&ad=&clid=&cs=Line&spb=&imt=referrer&"
      "isattrib=0&isinstattrib=0&isassist=0\n";
  // const char* testString = "11111 itv=hasoffers&clid=aaa.bbb.ccc";

  char* pResult = NULL;
  size_t resultSize = 0;
  pResult = pb2tsv(testString, strlen(testString), &resultSize);
  if (pResult) {
    printf("%s\n", pResult);
    free(pResult);
  } else {
    printf("Parse error.\n");
  }
  return 0;
}
#endif
