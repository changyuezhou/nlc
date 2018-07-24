#include "encoders.h"
#include "../log_api.h"

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
bool judge_kvm(const struct KeyValueMsg* kvm) {
  return kvm->value ? true : false;
}

encoder_type identity = &identity_encoder;

encoder_type protobuf = &encode_json2pb;

encoder_type quote = &quote_encoder;

encoder_type pb2tsvEncoder = &pb2tsv;

encoder_type dynamic = &encode_json2pb_dynamic;

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

encoder_type init_encoder_by_type(const char* option) {
  if(option) {
    if (strncmp(option, protobuf_conf_name, sizeof(protobuf_conf_name)) == 0) {
      return protobuf;
    } else if (strncmp(option, identity_conf_name, sizeof(identity_conf_name)) == 0) {
      return identity;
    } else if (strncmp(option, quote_conf_name, sizeof(quote_conf_name)) == 0) {
      return quote;
    } else if (strncmp(option, protobuf_dynamic_conf_name, sizeof(protobuf_dynamic_conf_name)) == 0) {
      //printf("Dynamic protobuf encoder selected\n");
      lts_public_log_debug("Dynamic protobuf encoder selected!");
      init_protobuf_by_pbfile();
      return dynamic;
    } else if (strncmp(option, url_escape_conf_name, sizeof(url_escape_conf_name)) == 0) {
      return url_escape;
    } else if (strncmp(option, pb2tsv_conf_name, sizeof(pb2tsv_conf_name)) == 0) {
      return pb2tsvEncoder;
    }
  }

  return NULL;
}
