#pragma once
#include <jansson.h>
#include <avro.h>

/*
 *   Encodes a c-string containing json data of size n.
 *
 *   Returns avro byte array of size j. On failure returns NULL.
 */

char* encode_json_to_avro(char* json, size_t n, const avro_schema_t* schema,
                          size_t* j);