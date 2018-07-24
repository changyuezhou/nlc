#pragma once
#include <librdkafka/rdkafka.h>
#include <stdio.h>

/**
 *
 *
 */


/*
 * Logging macros
 */

#define NLC_DEBUG 1
#define NLC_INFO 2
#define NLC_ERROR 3
#define NMN(lvl) (lvl == NLC_INFO) ? "INFO" : (lvl == NLC_ERROR) ? "ERROR" : "DEBUG"
#define CLR(lvl) (lvl == NLC_INFO) ? 32 : (lvl == NLC_ERROR) ? 31 : 36

#define out(lvl) (lvl == NLC_DEBUG || lvl == NLC_ERROR) ? stderr : stdout 
#define lts_print_log(lvl, ...) do { if(lvl >= conf.log_level) {                                                 \
                                          fprintf(out(lvl), "\x1b[1;%dm:: %s :: %s :: ", CLR(lvl), NMN(lvl), __TIMESTAMP__);   \
                                          fprintf(out(lvl), __VA_ARGS__);                                        \
                                        }                                                                        \ 
                                    } while (0)

#define lts_log_debug(...) lts_print_log(NLC_DEBUG, __VA_ARGS__)
#define lts_log_info(...) lts_print_log(NLC_INFO, __VA_ARGS__)
#define lts_log_error(...) lts_print_log(NLC_ERROR, __VA_ARGS__)


#define EXT_RESERVE 10  // characters reserved for prog/comp file extension

struct conf {
  /* lts config */
  char* comp_ext;
  size_t comp_ext_len;
  char* prog_ext;
  size_t prog_ext_len;
  char* watch_dir;
  size_t watch_dir_len;

  size_t max_log_path_size;
  size_t max_path_size;
  size_t max_line_length;

  int run;

  int daemonize;

  char* status_file;

  char* props;

  char* encoder;

  char* proto_type_name;
  char* proto_type_folder;
  char* proto_key_type_name;

  /* Kafka config */
  int partition;
  char* topic;

  /* File check backoff */
  unsigned long max_backoff; //in ms
  unsigned long incr_backoff; //in ms

  char* logname;
  int log_level;
  int log_to;
  int log_rate;        /* Maximum log rate per minute. */
  int log_rate_period; /* Log rate limiting period */

  int log_kafka_msg_error; /* Log Kafka message delivery errors*/

  rd_kafka_conf_t* rk_conf;
  rd_kafka_topic_conf_t* topic_conf;
};

extern struct conf conf;

/* Conf.c */
int read_config_file(const char* path);
void display_pb_cofig();
void init_default_config();
