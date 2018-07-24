#pragma once

#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "nlc.h"
#include "encoders/encoders.h"

int send_to_kafka(rd_kafka_t* rk, rd_kafka_topic_t* rkt, int32_t partition,
                  struct KeyValueMsg* kvm);

long int read_line_status(FILE** progfd);

void write_line_status(int status, fpos_t* pos, FILE** progfd, bool resetpos);

void open_log_prog_files(FILE** logfd, FILE** progfd, char* cur_file,
                         size_t cur_file_size);

void rename_and_close_files(FILE** logfd, FILE** progfd, char* cur_file);

/* char* rgets(char* buf, size_t size, int fp); */

int check_rotate(char* buf, size_t buf_size, const char* dir_path);

int fgets_wrapper(char* dst, size_t max, size_t* read_state, FILE* fp);

void ms2ts(struct timespec* ts, unsigned long ms);
