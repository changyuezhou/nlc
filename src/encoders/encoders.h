#pragma once
#include "identity.h"
#include "protobuf.h"
#include "quote.h"
#include "pb2tsv.h"
#include "url_escape.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/*
*	Encodes original logline
* input:
*	const char* == original log line
* size_t == original log line strlen
* return:
* KeyValueMsg, where value and message is allocated with malloc
*/

struct KeyValueMsg {
  void* value;
  void* key;
  void* _private;
  size_t value_size;
  size_t key_size;
};

extern struct KeyValueMsg default_kvm;


typedef struct KeyValueMsg (*encoder_type)(const char*, size_t);

void free_kvm(struct KeyValueMsg* kvm);

bool valid_kvm(const struct KeyValueMsg* kvm);

void print_encoder_names();

encoder_type select_encoder(const char* option);
