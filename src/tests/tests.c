#include <stdio.h>
#include <stdlib.h>
#include "encoder.h"

#define MAX_SCHEMA_LEN ((off_t)1024 * 1024)

char *read_schema_file(char *file_name) {
  FILE *schema_file = fopen(file_name, "rt");
  if (errno != 0) {
    fprintf(stderr, "Could not find or access file: %s\n", file_name);
    return 0;
  }

  // Get file size
  fseek(schema_file, 0, SEEK_END);
  off_t file_size = ftell(schema_file);
  fseek(schema_file, 0, SEEK_SET);

  if (file_size == 0) {
    fprintf(stderr, "Empty schema file: %s\n", file_name);
    return 0;
  }

  if (file_size > MAX_SCHEMA_LEN) {
    fprintf(stderr,
            "Schema file size is too big: %lld bytes > %lld maximum supported "
            "length\n",
            file_size, MAX_SCHEMA_LEN);
    return 0;
  }

  // Allocate buffer for the schema and read the data
  char *buf = (char *)malloc(file_size);
  fread(buf, 1, file_size, schema_file);
  fclose(schema_file);

  return buf;
}

int main() {
  FILE *jsonfp;
  char *buf;
  char *schema_arg = NULL;
  avro_schema_t schema;
  char *result;

  schema_arg = read_schema_file("schema.avro");
  if (avro_schema_from_json_length(schema_arg, strlen(schema_arg), &schema)) {
    fprintf(stderr, "ERROR: Unable to parse schema: '%s'\n", schema_arg);
    exit(EXIT_FAILURE);
  }

  size_t size = 4048;
  buf = (char *)malloc(size);
  size_t j;
  if ((jsonfp = fopen("test.json", "r"))) {
    if (getline(&buf, &size, jsonfp) > 0) {
      // puts(buf);
    }
  }
  result = encode_json_to_avro(buf, size, &schema, &j);

  avro_reader_t reader = NULL;

  if (!(reader = avro_reader_memory(result, j))) {
    printf("Error creating memory reader\n");
    exit(EXIT_FAILURE);
  }

  avro_value_t result_value;
  avro_value_iface_t *iface = avro_generic_class_from_schema(schema);
  avro_generic_value_new(iface, &result_value);

  if (avro_value_read(reader, &result_value)) {
    printf("Error reading from reader");
    exit(EXIT_FAILURE);
  }

  char *result_jsoncstr;
  avro_value_to_json(&result_value, 0, &result_jsoncstr);
  printf("%s\n", result_jsoncstr);

  return 0;
}