#pragma once
#include "encoders.h"

// pb2tsv will convert one line of the log file from AD-Postback HTTP request
// into a TSV(tab-seperated-value) string.
// The TSV will contain the following field.
//      1. create_at
//      2. action
//      3. vendor
//      4. app_pkg_name
//      5. app_pkg_ver
//      6. ip
//      7. gaid
//      8. idfa
//      9. ad_id
//      10. license_key
//      11. language
//      12. country
//      13. user_agent
//      14. isinstattrib
//      15. isassist
//      16. campaign
//
// If the field contains no value from the http request, the value will
// be filled with `\N`.
// PARAMETER:
//      result        pb2tsv will allocate memory via malloc. The address
//                      will be stored in KeyValueMsg.value. The caller should use
//                      free to release the memory.
//      pLine           The log line to be convert.
//      n               The size of pLine.
// RETURN:
//      pb2tsv will allocate memory via malloc. The address
//      will be stored in KeyValueMsg.value. The caller should use
//      free to release the memory.
struct KeyValueMsg pb2tsv(const char* pLine, size_t n);
