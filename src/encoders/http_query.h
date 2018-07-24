#pragma once

#ifdef __cplusplus
#include <map>
#include <string>

typedef std::map<std::string, std::string> HttpQuery;

/*
 * decode_http_query decodes the query string of an http request url.
 * The "HttpQuery" result will contains all the names and the values of the
 * query.
 */
bool decode_http_query(HttpQuery& result, const char* pQueryString);

/*
 * decode_url_string decodes the string which is URL encoded.
 * pResult  The decoded string will be here.
 * pSize    As the input parameter, it specify how many characters pResult can
 * store. It includes the NULL terminator.
 *          As the output parameter, it reports the size pResult should have to
 * store the result.
 * pUrlString   The string to be decoded.
 * urlLen       The length of pUrlString. The NULL terminator is not included.
 */
bool decode_url(char* pResult, size_t* pSize, const char* pUrlString,
                size_t urlLen);

/*
 * Convert char to int
 * return -1 if invalid
 */
extern "C" {
int char2int(char);
}
#endif
#ifndef __cplusplus
int char2int(char);
#endif
