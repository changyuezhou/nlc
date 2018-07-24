#pragma once
#include "encoders.h"
#include <stdlib.h>

/*
 *   Encodes a c-string containing json data of size n.
 *
 *   Returns avro byte array of size j. On failure returns NULL.
 */

/*check for c++ compiler*/
#ifdef __cplusplus
extern "C" {
struct KeyValueMsg encode_json2pb(const char* json, size_t n);
struct KeyValueMsg encode_json2pb_dynamic(const char* json, size_t n);
const char* decode_pb2json(const char* protobuf, size_t n);
void init_protobuf_by_pbfile();
}
#endif
#ifndef __cplusplus
struct KeyValueMsg encode_json2pb(const char* json, size_t n);
struct KeyValueMsg encode_json2pb_dynamic(const char* json, size_t n);
const char* decode_pb2json(const char* protobuf, size_t n);
void init_protobuf_by_pbfile();
#endif
