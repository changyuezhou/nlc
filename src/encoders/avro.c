
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "encoder.h"

int schema_traverse(const avro_schema_t schema, json_t *json, json_t *dft,
                    avro_value_t *current_val, int quiet, int strjson,
                    size_t max_str_sz);

/*
 *   Encodes a c-string containing json data of size n.
 *
 *   Returns avro byte array of size j. On failure returns NULL.
 *   Caller is responsible for freeing the memory when done with it.
 */

char *encode_json_to_avro(char *json_cstr, size_t n,
                          const avro_schema_t *schema, size_t *j) {
  char *buf = NULL;

  // Prepare json type
  json_error_t err;
  json_t *json = NULL;
  int strjson = 0, max_str_sz = 0;  // strjson = dump unknown fields as strings
                                    // (0=off), max_str_sz 0=unlimited str size
  int quiet = 0;                    // 0 = print error messages
  size_t avro_size;
  avro_writer_t writer;

  if (!(json = json_loadb(json_cstr, n, JSON_DISABLE_EOF_CHECK, &err))) {
    fprintf(stderr, "Failed to parse json string");
    return NULL;
  }

  // Prepare avro record
  avro_value_t record;
  avro_value_iface_t *iface = avro_generic_class_from_schema(*schema);
  avro_generic_value_new(iface, &record);

  // Traverse schema;  encoding json fields to avro binary format.
  if (schema_traverse(*schema, json, NULL, &record, quiet, strjson,
                      max_str_sz)) {
    fprintf(stderr, "Error processing record %zu, skipping...\n", n);
    return NULL;
  }

  // Calculate byte array size
  if (avro_value_sizeof(&record, &avro_size)) {
    fprintf(stderr, "Error while calculating record size\n");
    return NULL;
  }

  // Allocate memory for avro binary
  if (!(buf = malloc(avro_size))) {
    fprintf(stderr, "Error while allocating memory for serialized output\n");
    return NULL;
  }

  // Prepare avro memory writer
  if (!(writer = avro_writer_memory(buf, avro_size))) {
    fprintf(stderr, "Error while creating memory writer\n");
    return NULL;
  }

  // Write value to ouput
  if (avro_value_write(writer, &record)) {
    fprintf(stderr, "Error, value too large for buffer\n");
    return NULL;
  }

  // Update size
  *j = avro_size;

  // Memory management
  json_object_clear(json);
  json_decref(json);
  avro_writer_free(writer);
  avro_value_iface_decref(iface);
  avro_value_decref(&record);

  return buf;
}

/*
 *  Code from https://github.com/grisha/json2avro
 *   Apache License v2.0
 */

int schema_traverse(const avro_schema_t schema, json_t *json, json_t *dft,
                    avro_value_t *current_val, int quiet, int strjson,
                    size_t max_str_sz) {
  // json = json ? json : dft;
  if (!json) {
    fprintf(stderr, "ERROR: Avro schema does not match JSON\n");
    return 1;
  }

  switch (schema->type) {
    case AVRO_RECORD: {
      if (!json_is_object(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON object for Avro record, got something "
                  "else\n");
        return 1;
      }

      int len = avro_schema_record_size(schema), i;
      for (i = 0; i < len; i++) {
        const char *name = avro_schema_record_field_name(schema, i);
        avro_schema_t field_schema =
            avro_schema_record_field_get_by_index(schema, i);

        json_t *json_val = json_object_get(json, name);

        // This is to get default value in case a json field is missing;
        // It requires modification to standard c-avro, and I dont think we need
        // it
        // json_t *dft = avro_schema_record_field_default_get_by_index(schema,
        // i);

        avro_value_t field;
        avro_value_get_by_index(current_val, i, &field, NULL);

        if (schema_traverse(field_schema, json_val, dft, &field, quiet, strjson,
                            max_str_sz))
          return 1;
      }
    } break;

    case AVRO_LINK:
      /* TODO */
      fprintf(stderr, "ERROR: AVRO_LINK is not implemented\n");
      return 1;
      break;

    case AVRO_STRING:
      if (!json_is_string(json)) {
        if (json && strjson) {
          /* -j specified, just dump the remaining json as string */
          char *js =
              json_dumps(json, JSON_COMPACT | JSON_SORT_KEYS | JSON_ENCODE_ANY);
          if (max_str_sz && (strlen(js) > max_str_sz))
            js[max_str_sz] =
                0; /* truncate the string - this will result in invalid JSON! */
          avro_value_set_string(current_val, js);
          free(js);
          break;
        }
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON string for Avro string, got something "
                  "else\n");
        return 1;
      } else {
        const char *js = json_string_value(json);
        if (max_str_sz && (strlen(js) > max_str_sz)) {
          /* truncate the string */
          char *jst = malloc(strlen(js));
          strcpy(jst, js);
          jst[max_str_sz] = 0;
          avro_value_set_string(current_val, jst);
          free(jst);
        } else
          avro_value_set_string(current_val, js);
      }
      break;

    case AVRO_BYTES:
      if (!json_is_string(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON string for Avro string, got something "
                  "else\n");
        return 1;
      }
      /* NB: Jansson uses null-terminated strings, so embedded nulls are NOT
         supported, not even escaped ones */
      const char *s = json_string_value(json);
      avro_value_set_bytes(current_val, (void *)s, strlen(s));
      break;

    case AVRO_INT32:
      if (!json_is_integer(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON integer for Avro int, got something "
                  "else\n");
        return 1;
      }
      avro_value_set_int(current_val, json_integer_value(json));
      break;

    case AVRO_INT64:
      if (!json_is_integer(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON integer for Avro long, got something "
                  "else\n");
        return 1;
      }
      avro_value_set_long(current_val, json_integer_value(json));
      break;

    case AVRO_FLOAT:
      if (!json_is_real(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON real for Avro float, got something "
                  "else\n");
        return 1;
      }
      avro_value_set_float(current_val, json_real_value(json));
      break;

    case AVRO_DOUBLE:
      if (!json_is_real(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON real for Avro double, got something "
                  "else\n");
        return 1;
      }
      avro_value_set_double(current_val, json_real_value(json));
      break;

    case AVRO_BOOLEAN:
      if (!json_is_boolean(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON boolean for Avro boolean, got "
                  "something else\n");
        return 1;
      }
      avro_value_set_boolean(current_val, json_is_true(json));
      break;

    case AVRO_NULL:
      if (!json_is_null(json)) {
        if (!quiet)
          fprintf(
              stderr,
              "ERROR: Expecting JSON null for Avro null, got something else\n");
        return 1;
      }
      avro_value_set_null(current_val);
      break;

    case AVRO_ENUM:
      // TODO ???
      fprintf(stderr, "ERROR: ENUM not supported\n");
      return 1;
      break;

    case AVRO_ARRAY:
      if (!json_is_array(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON array for Avro array, got something "
                  "else\n");
        return 1;
      } else {
        int i, len = json_array_size(json);
        avro_schema_t items = avro_schema_array_items(schema);
        avro_value_t val;
        for (i = 0; i < len; i++) {
          avro_value_append(current_val, &val, NULL);
          if (schema_traverse(items, json_array_get(json, i), NULL, &val, quiet,
                              strjson, max_str_sz))
            return 1;
        }
      }
      break;

    case AVRO_MAP:
      if (!json_is_object(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON object for Avro map, got something "
                  "else\n");
        return 1;
      } else {
        avro_schema_t values = avro_schema_map_values(schema);
        void *iter = json_object_iter(json);
        avro_value_t val;
        while (iter) {
          avro_value_add(current_val, json_object_iter_key(iter), &val, 0, 0);
          if (schema_traverse(values, json_object_iter_value(iter), NULL, &val,
                              quiet, strjson, max_str_sz))
            return 1;
          iter = json_object_iter_next(json, iter);
        }
      }
      break;

    case AVRO_UNION: {
      int i;
      avro_value_t branch;
      for (i = 0; i < avro_schema_union_size(schema); i++) {
        avro_value_set_branch(current_val, i, &branch);
        avro_schema_t type = avro_schema_union_branch(schema, i);
        if (!schema_traverse(type, json, NULL, &branch, 1, strjson, max_str_sz))
          break;
      }
      if (i == avro_schema_union_size(schema)) {
        fprintf(
            stderr,
            "ERROR: No type in the Avro union matched the JSON type we got\n");
        return 1;
      }
      break;
    }
    case AVRO_FIXED:
      if (!json_is_string(json)) {
        if (!quiet)
          fprintf(stderr,
                  "ERROR: Expecting JSON string for Avro fixed, got something "
                  "else\n");
        return 1;
      }
      /* NB: Jansson uses null-terminated strings, so embedded nulls are NOT
         supported, not even escaped ones */
      const char *f = json_string_value(json);
      if (!avro_value_set_fixed(current_val, (void *)f, strlen(f))) {
        fprintf(stderr, "ERROR: Setting Avro fixed value FAILED\n");
        return 1;
      }
      break;

    default:
      fprintf(stderr, "ERROR: Unknown type: %d\n", schema->type);
      return 1;
  }
  return 0;
}