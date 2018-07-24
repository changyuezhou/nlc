#include "encoders.h"

struct KeyValueMsg default_kvm = { NULL, NULL, NULL, 0, 0 };

void free_kvm(struct KeyValueMsg* kvm) {
  if(kvm) {
    if (kvm->value) free(kvm->value);
    if (kvm->key) free(kvm->key);
    if (kvm->_private) free(kvm->_private);
    free(kvm);
  }
}
/*
 * kvm is only valid if value is not null
 *
 */
bool valid_kvm(const struct KeyValueMsg* kvm) {
  return kvm->value ? true : false;
}

encoder_type identity = &identity_encoder;

encoder_type protobuf = &encode_json_to_protobuf;

encoder_type quote = &quote_encoder;

encoder_type pb2tsvEncoder = &pb2tsv;

encoder_type dynamic = &encode_json_to_protobuf_dynamic;

encoder_type url_escape = &url_escape_encoder;

char protobuf_conf_name[] = "dsp_protobuf";
char protobuf_dynamic_conf_name[] = "dynamic_protobuf";
char identity_conf_name[] = "identity";
char quote_conf_name[] = "quote";
char pb2tsv_conf_name[] = "pb2tsv";
char url_escape_conf_name[] = "url_escape";

void print_encoder_names() {
  printf("%s\n", protobuf_conf_name);
  printf("%s\n", protobuf_dynamic_conf_name);
  printf("%s\n", identity_conf_name);
  printf("%s\n", quote_conf_name);
  printf("%s\n", pb2tsv_conf_name);
  printf("%s\n", url_escape_conf_name);
}

encoder_type select_encoder(const char* option) {
  if(option) {
    if (strncmp(option, protobuf_conf_name, sizeof(protobuf_conf_name)) == 0) {
      printf("DSP Protobuf encoder selected\n");
      return protobuf;
    } else if (strncmp(option, identity_conf_name, sizeof(identity_conf_name)) == 0) {
      printf("Identity encoder selected\n");
      return identity;
    } else if (strncmp(option, quote_conf_name, sizeof(quote_conf_name)) == 0) {
      printf("Quote encoder selected\n");
      return quote;
    } else if (strncmp(option, protobuf_dynamic_conf_name, sizeof(protobuf_dynamic_conf_name)) == 0) {
      printf("Dynamic protobuf encoder selected\n");
      initalize_dynamic_protobuf();
      return dynamic;
    } else if (strncmp(option, url_escape_conf_name, sizeof(url_escape_conf_name)) == 0) {
      printf("UrlEscape encoder selected\n");
      return url_escape;
    } else if (strncmp(option, pb2tsv_conf_name, sizeof(pb2tsv_conf_name)) == 0) {
      printf("Pb2tsv encoder selected.\n");
      return pb2tsvEncoder;
    }
  }

  return NULL;
}
