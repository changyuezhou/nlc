/*
 * Copyright (c) 2013 Pavel Shramov <shramov@mexmat.net>
 *
 * json2pb is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * https://github.com/shramov/json2pb
 */

#include <errno.h>
#include <jansson.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <iostream>

#include "encoders.h"
#include "logspec.pb.h"
#include "http_query.h"

// for global config
#include "../nlc.h"

#include <stdexcept>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstring>

namespace {
#include "bin2ascii.h"
}

using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::Reflection;
using google::protobuf::compiler::Importer;
using google::protobuf::compiler::DiskSourceTree;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::DescriptorProto;

class ErrorPrinter
    : public google::protobuf::compiler::MultiFileErrorCollector {
 public:
  void AddError(const std::string &filename, int line, int column,
                const std::string &msg) {
    fprintf(stderr,
            "Parse errror in %s, at line %i column %i : Error msg: %s\n",
            filename.c_str(), line, column, msg.c_str());
  }
};

static auto dmf = new DynamicMessageFactory();
static auto dst = new DiskSourceTree();
static auto err_reporter = new ErrorPrinter();
static auto imp = new Importer(dst, err_reporter);
static const Message* msg_builder = NULL;
static const FieldDescriptor* key_field = NULL;

struct json_autoptr {
  json_t *ptr;

  json_autoptr(json_t *json) : ptr(json) {}

  ~json_autoptr() {
    if (ptr) json_decref(ptr);
  }

  json_t *release() {
    json_t *tmp = ptr;
    ptr = 0;
    return tmp;
  }
};

class j2pb_error : public std::exception {
  std::string _error;

 public:
  j2pb_error(const std::string &e) : _error(e) {}

  j2pb_error(const FieldDescriptor *field, const std::string &e)
      : _error(field->name() + ": " + e) {}

  virtual ~j2pb_error() throw(){};

  virtual const char *what() const throw() { return _error.c_str(); };
  virtual std::string error() const throw() { return _error; };
};

static json_t *_pb2json(const Message &msg);

struct fieldv {
  size_t size;
  void*  data;
};

static struct fieldv _field2fieldv(const Message &msg, const FieldDescriptor *field) {
  const Reflection *ref = msg.GetReflection();
  const bool repeated = field->is_repeated();
  if (repeated) throw 10;
  struct fieldv result;
  result.size = 0;
  result.data = NULL;
  switch (field->cpp_type()) {
#define _CONVERT(type, ctype, sfunc)                                  \
  case FieldDescriptor::type: {                                              \
    const ctype value = ref->sfunc(msg, field);                              \
    result.size = sizeof(ctype);                                             \
    result.data = malloc(result.size);                                       \
    memcpy(result.data, &value, result.size);                                 \
    break;                                                                   \
  }

    _CONVERT(CPPTYPE_DOUBLE, double, GetDouble);
    _CONVERT(CPPTYPE_FLOAT, float, GetFloat);
    _CONVERT(CPPTYPE_INT64, int64_t, GetInt64);
    _CONVERT(CPPTYPE_UINT64, uint64_t, GetUInt64);
    _CONVERT(CPPTYPE_INT32, int32_t, GetInt32);
    _CONVERT(CPPTYPE_UINT32, uint32_t, GetUInt32);
    _CONVERT(CPPTYPE_BOOL, bool, GetBool);
#undef _CONVERT
    case FieldDescriptor::CPPTYPE_STRING: {
      std::string scratch;
      const std::string &value = ref->GetStringReference(msg, field, &scratch);
      if (field->type() == FieldDescriptor::TYPE_BYTES) {
        auto b64data = b64_encode(value);
        result.size = b64data.size();
        result.data = malloc(result.size);
        memcpy(result.data, b64data.c_str(), result.size);
      } else {
        result.size = value.size();
        result.data = malloc(result.size);
        memcpy(result.data, value.c_str(), result.size);
      }
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      const Message &mf = ref->GetMessage(msg, field);
      result.size = mf.ByteSize();
      result.data = malloc(result.size);
      mf.SerializeToArray(result.data, result.size);
      break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      throw 10;
      break;
    }
    default:
      break;
  }
  if (!result.data) throw 10;
  return result;
}

static json_t *_field2json(const Message &msg, const FieldDescriptor *field,
                           size_t index) {
  const Reflection *ref = msg.GetReflection();
  const bool repeated = field->is_repeated();
  json_t *jf = 0;
  switch (field->cpp_type()) {
#define _CONVERT(type, ctype, fmt, sfunc, afunc)                             \
  case FieldDescriptor::type: {                                              \
    const ctype value =                                                      \
        (repeated) ? ref->afunc(msg, field, index) : ref->sfunc(msg, field); \
    jf = fmt(value);                                                         \
    break;                                                                   \
  }

    _CONVERT(CPPTYPE_DOUBLE, double, json_real, GetDouble, GetRepeatedDouble);
    _CONVERT(CPPTYPE_FLOAT, double, json_real, GetFloat, GetRepeatedFloat);
    _CONVERT(CPPTYPE_INT64, json_int_t, json_integer, GetInt64,
             GetRepeatedInt64);
    _CONVERT(CPPTYPE_UINT64, json_int_t, json_integer, GetUInt64,
             GetRepeatedUInt64);
    _CONVERT(CPPTYPE_INT32, json_int_t, json_integer, GetInt32,
             GetRepeatedInt32);
    _CONVERT(CPPTYPE_UINT32, json_int_t, json_integer, GetUInt32,
             GetRepeatedUInt32);
    _CONVERT(CPPTYPE_BOOL, bool, json_boolean, GetBool, GetRepeatedBool);
#undef _CONVERT
    case FieldDescriptor::CPPTYPE_STRING: {
      std::string scratch;
      const std::string &value =
          (repeated)
              ? ref->GetRepeatedStringReference(msg, field, index, &scratch)
              : ref->GetStringReference(msg, field, &scratch);
      if (field->type() == FieldDescriptor::TYPE_BYTES)
        jf = json_string(b64_encode(value).c_str());
      else
        jf = json_string(value.c_str());
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      const Message &mf = (repeated)
                              ? ref->GetRepeatedMessage(msg, field, index)
                              : ref->GetMessage(msg, field);
      jf = _pb2json(mf);
      break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      const EnumValueDescriptor *ef =
          (repeated) ? ref->GetRepeatedEnum(msg, field, index)
                     : ref->GetEnum(msg, field);

      jf = json_integer(ef->number());
      break;
    }
    default:
      break;
  }
  if (!jf) throw j2pb_error(field, "Fail to convert to json");
  return jf;
}

static json_t *_pb2json(const Message &msg) {
  const Descriptor *d = msg.GetDescriptor();
  const Reflection *ref = msg.GetReflection();
  if (!d || !ref) return 0;

  json_t *root = json_object();
  json_autoptr _auto(root);

  std::vector<const FieldDescriptor *> fields;
  ref->ListFields(msg, &fields);

  for (size_t i = 0; i != fields.size(); i++) {
    const FieldDescriptor *field = fields[i];

    json_t *jf = 0;
    if (field->is_repeated()) {
      size_t count = ref->FieldSize(msg, field);
      if (!count) continue;

      json_autoptr array(json_array());
      for (size_t j = 0; j < count; j++)
        json_array_append_new(array.ptr, _field2json(msg, field, j));
      jf = array.release();
    } else if (ref->HasField(msg, field))
      jf = _field2json(msg, field, 0);
    else
      continue;

    const std::string &name =
        (field->is_extension()) ? field->full_name() : field->name();
    json_object_set_new(root, name.c_str(), jf);
  }
  return _auto.release();
}

static void _json2pb(Message &msg, json_t *root);

static void _json2field(Message &msg, const FieldDescriptor *field,
                        json_t *jf) {
  const Reflection *ref = msg.GetReflection();
  const bool repeated = field->is_repeated();

  //Check for optional null values
  if (json_is_null(jf) && field->is_optional()) return;
  
  json_error_t error;

  switch (field->cpp_type()) {
#define _SET_OR_ADD(sfunc, afunc, value) \
  do {                                   \
    if (repeated)                        \
      ref->afunc(&msg, field, value);    \
    else                                 \
      ref->sfunc(&msg, field, value);    \
  } while (0)

#define _CONVERT(type, ctype, fmt, sfunc, afunc)                               \
  case FieldDescriptor::type: {                                                \
    ctype value;                                                               \
    int r = json_unpack_ex(jf, &error, JSON_STRICT, fmt, &value);              \
    if (r)                                                                     \
      throw j2pb_error(field, std::string("Failed to unpack: ") + error.text); \
    _SET_OR_ADD(sfunc, afunc, value);                                          \
    break;                                                                     \
  }

    _CONVERT(CPPTYPE_DOUBLE, double, "F", SetDouble, AddDouble);
    _CONVERT(CPPTYPE_FLOAT, double, "F", SetFloat, AddFloat);
    _CONVERT(CPPTYPE_INT64, json_int_t, "I", SetInt64, AddInt64);
    _CONVERT(CPPTYPE_UINT64, json_int_t, "I", SetUInt64, AddUInt64);
    _CONVERT(CPPTYPE_INT32, json_int_t, "I", SetInt32, AddInt32);
    _CONVERT(CPPTYPE_UINT32, json_int_t, "I", SetUInt32, AddUInt32);
    _CONVERT(CPPTYPE_BOOL, int, "b", SetBool, AddBool);

    case FieldDescriptor::CPPTYPE_STRING: {
      if (!json_is_string(jf)) throw j2pb_error(field, "Not a string");
      const char *value = json_string_value(jf);
      if (field->type() == FieldDescriptor::TYPE_BYTES)
        _SET_OR_ADD(SetString, AddString, b64_decode(value));
      else
        _SET_OR_ADD(SetString, AddString, value);
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      Message *mf = (repeated) ? ref->AddMessage(&msg, field)
                               : ref->MutableMessage(&msg, field);
      _json2pb(*mf, jf);
      break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      const EnumDescriptor *ed = field->enum_type();
      const EnumValueDescriptor *ev = 0;
      if (json_is_integer(jf)) {
        ev = ed->FindValueByNumber(json_integer_value(jf));
      } else if (json_is_string(jf)) {
        ev = ed->FindValueByName(json_string_value(jf));
      } else
        throw j2pb_error(field, "Not an integer or string");
      if (!ev) throw j2pb_error(field, "Enum value not found");
      _SET_OR_ADD(SetEnum, AddEnum, ev);
      break;
    }
    default:
      break;
  }
}

static void _json2pb(Message &msg, json_t *root) {
  const Descriptor *d = msg.GetDescriptor();
  const Reflection *ref = msg.GetReflection();
  if (!d || !ref) throw j2pb_error("No descriptor or reflection");

  for (void *i = json_object_iter(root); i;
       i = json_object_iter_next(root, i)) {
    const char *name = json_object_iter_key(i);
    json_t *jf = json_object_iter_value(i);

    const FieldDescriptor *field = d->FindFieldByName(name);
    if (!field) field = ref->FindKnownExtensionByName(name);
    // field = d->file()->FindExtensionByName(name);

    if (!field) throw j2pb_error("Unknown field: " + std::string(name));

    if (field->is_repeated()) {
      if (!json_is_array(jf)) throw j2pb_error(field, "Not array");
      for (size_t j = 0; j < json_array_size(jf); j++)
        _json2field(msg, field, json_array_get(jf, j));
    } else
      _json2field(msg, field, jf);
  }
}

void json2pb(Message &msg, const char *buf, size_t size) {
  json_t *root;
  json_error_t error;

  root = json_loadb(buf, size, 0, &error);

  if (!root) throw j2pb_error(std::string("Load failed: ") + error.text);

  json_autoptr _auto(root);

  if (!json_is_object(root)) throw j2pb_error("Malformed JSON: not an object");

  _json2pb(msg, root);
}

int json_dump_std_string(const char *buf, size_t size, void *data) {
  std::string *s = (std::string *)data;
  s->append(buf, size);
  return 0;
}

std::string pb2json(const Message &msg) {
  std::string r;

  json_t *root = _pb2json(msg);
  json_autoptr _auto(root);
  json_dump_callback(root, json_dump_std_string, &r, 0);
  return r;
}

/*
 *      encoder API
 *
 *
 *
 *
 *
 *
 */

/*
 *  encode_json_to_protobuf
 *
 *
 *
 */
 extern "C" {
  struct KeyValueMsg encode_json_to_protobuf(const char *json, size_t strlength) {
    logspec::LogLine msg;
    struct KeyValueMsg kvm = default_kvm;
    bool success = false;
    void *buffer = NULL;

    size_t decode_len = strlength + 1;
    char decoded[decode_len];

    if (decode_url(decoded, &decode_len, json, strlength)) {
      try {
        json2pb(msg, decoded, strlen(decoded));
        success = true;
      } catch (...) {
        success = false;
      }
    }
    if (success) {
      size_t size = msg.ByteSize();

      buffer = malloc(size);

      msg.SerializeToArray(buffer, size);

      // update return size
      kvm.value_size = size;
      kvm.value = buffer;
    }
    return kvm;
  }
}


static int proto_filter(const struct dirent *entry) {
  const char *result_prog = strstr(entry->d_name, ".proto");
  size_t length = strlen(entry->d_name); /* remove . and .. */
  return ((length > 2) && (result_prog ? 1 : 0));
}

void initalize_dynamic_protobuf() {
  struct dirent **namelist;
  int n, scandir_res = 0;
  assert(conf.proto_type_folder);

  dst->MapPath("", conf.proto_type_folder);
  printf("Scanning %s for .proto files\n", conf.proto_type_folder);

  scandir_res =
      scandir(conf.proto_type_folder, &namelist, proto_filter, alphasort);
  n = scandir_res;
  if (n < 0) {
    fprintf(stderr, "failed to scan protobuf folder %s\n",
            conf.proto_type_folder);
    exit(EXIT_FAILURE);
  } else if (n > 0) {
    while (n--) {
      printf("Importing %s\n", namelist[n]->d_name);
      auto fd = imp->Import(namelist[n]->d_name);
      if (fd == NULL)
        fprintf(stderr, "Failed to import %s\n", namelist[n]->d_name);
      free(namelist[n]);
    }
    free(namelist);
  }

  auto pool = imp->pool();
  auto msg_disc = pool->FindMessageTypeByName(conf.proto_type_name);
  if (msg_disc == NULL) {
    fprintf(stderr, "Failed to find type %s\n", conf.proto_type_name);
    exit(EXIT_FAILURE);
  }
  msg_builder = dmf->GetPrototype(msg_disc);

  if(conf.proto_key_type_name) {
    key_field = msg_builder->GetDescriptor()->FindFieldByName(conf.proto_key_type_name);
    if (key_field == NULL) {
      fprintf(stderr, "Failed to find key type %s\n", conf.proto_key_type_name);
      exit(EXIT_FAILURE);
    }
  }
}

extern "C" {
  struct KeyValueMsg encode_json_to_protobuf_dynamic(const char *json, size_t strlength) {
    Message *msg = msg_builder->New();
    struct KeyValueMsg kvm = default_kvm;
    bool success = false;
    void *buffer = NULL;

    size_t decode_len = strlength + 1;
    char decoded[decode_len];

    if (decode_url(decoded, &decode_len, json, strlength)) {
      try {
        json2pb(*msg, decoded, strlen(decoded));
        success = true;
      } catch (j2pb_error error) {
        success = false;
        std::cout << error.error() << std::endl;
      }
    }
    if (success) {
      if (key_field) {
        auto key = _field2fieldv(*msg, key_field);
        kvm.key = key.data;
        kvm.key_size = key.size;
      }
      
      size_t size = msg->ByteSize();

      buffer = malloc(size);
      msg->SerializeToArray(buffer, size);

      // update result value and size
      kvm.value = buffer;
      kvm.value_size = size;
    }

    delete msg;
    return kvm;
  }
}
const char *decode_protobuf_to_json(const char *protobuf, size_t n) {
  logspec::LogLine msg;

  msg.ParseFromArray(protobuf, n);

  std::string json = pb2json(msg);

  return json.c_str();
}

const char *decode_protobuf_to_json_dynamic(const char *protobuf, size_t n) {
  Message *msg = msg_builder->New();

  bool result = msg->ParseFromArray(protobuf, n);

  if (!result)
    fprintf(stderr, "Failed to parse bytearray using protobuf typpe %s\n",
            conf.proto_type_name);

  std::string json = pb2json(*msg);

  return json.c_str();
}
