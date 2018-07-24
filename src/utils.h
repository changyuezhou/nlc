#pragma once

#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "nlc.h"
#include "encoders/encoders.h"

int send_to_server(rd_kafka_t* rk, rd_kafka_topic_t* rkt, int32_t partition,
                  struct KeyValueMsg* kvm);

long int fgets_line_status(FILE** progfd);

void fputs_line_status(int status, fpos_t* pos, FILE** progfd, bool resetpos);

void open_oldest_files(FILE** logfd, FILE** progfd, char* cur_file,
                         size_t cur_file_size);

void rename_and_close_files(FILE** logfd, FILE** progfd, char* cur_file);

/* char* rgets(char* buf, size_t size, int fp); */

int search_oldest_uncomplete(char* buf, size_t buf_size, const char* dir_path);

int fgets_file_data(char* dst, size_t max, size_t* read_state, FILE* fp);

void ms2timespec(struct timespec* ts, unsigned long ms);
