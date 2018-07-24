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
struct KeyValueMsg encode_json_to_protobuf(const char* json, size_t n);
struct KeyValueMsg encode_json_to_protobuf_dynamic(const char* json, size_t n);
const char* decode_protobuf_to_json(const char* protobuf, size_t n);
void initalize_dynamic_protobuf();
}
#endif
#ifndef __cplusplus
struct KeyValueMsg encode_json_to_protobuf(const char* json, size_t n);
struct KeyValueMsg encode_json_to_protobuf_dynamic(const char* json, size_t n);
const char* decode_protobuf_to_json(const char* protobuf, size_t n);
void initalize_dynamic_protobuf();
#endif
