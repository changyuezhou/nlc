#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <syslog.h>
#include <unistd.h>

#include "encoders/encoders.h"
#include "nlc.h"
#include "utils.h"

#ifndef VERSION
#define VERSION "no-verson-info"
#endif
#ifndef TAG
#define TAG "no-tag-info"
#endif
#ifndef BUILD_TIME
#define BUILD_TIME "no-build-time-info"
#endif

/* Global variables */
static rd_kafka_t *rk;
static rd_kafka_topic_t *rkt;
static int partition = RD_KAFKA_PARTITION_UA;

static FILE *logfd, *progressfd;
/* Total log line count across all log processed log file */
static unsigned long long global_line_count = 0;

/*
 *   Show command line usage help
 */

static encoder_type encoder_function;

void interrupt_callback_handler(int signum) {
  nlc_log_error("Second interrup signal, hard shutdown\n");
  exit(0);
}

void signal_callback_handler(int signum) {
  nlc_log_info("Caught signal %d\n", signum);
  nlc_log_info("Graceful shutdown started\n");
  conf.run = 0;
}

static void show_usage(char **argv) {
  nlc_log_error("Usage: %s -p <file.prop>"
                "\n"
                "librdkafka version %s (0x%08x)\n"
                "nlc version %s (tag: %s)\n"
                "build time: %s\n"
                "\n"
                " Options:\n"
                "  -p <file.prop>  nlc properties file (default: nlc.props)\n"
                "  -h Show this help\n"
                "  -H Display rdkafka properties infromation \n"
                "  -d Display loaded properties\n"
                "\n"
                "\n",
                argv[0], rd_kafka_version_str(), rd_kafka_version(), VERSION,
                TAG, BUILD_TIME);
}

/*
 * Kafka logger callback (optional)
 */

static void kafka_logger(const rd_kafka_t *rk, int level, const char *fac,
                         const char *buf) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  fprintf(stdout, "%u.%03u RDKAFKA-%i-%s: %s: %s\n", (int)tv.tv_sec,
          (int)(tv.tv_usec / 1000), level, fac, rd_kafka_name(rk), buf);
}

/*
 * Message delivery report callback.
 * Called once for each message.
 * This function is responsible for freeing memory.
 */

static void msg_delivered(rd_kafka_t *rk, void *payload, size_t len,
                          int error_code, void *opaque, void *msg_opaque) {
  struct KeyValueMsg *kvm = (struct KeyValueMsg *)msg_opaque;
  if (error_code) {
    nlc_log_error("%% Message delivery failed: %s\n Readding to kafka queue\n",
                  rd_kafka_err2str(error_code));
    send_to_kafka(rk, rkt, partition, kvm);
  } else {
    fpos_t *pos = (fpos_t *)kvm->_private;
    /* Write current line status to 1 (aka submitted) */
    write_line_status(1, pos, &progressfd, true);
    ++global_line_count;
    free_kvm(kvm);
  }
}

/*
 * Main function entry
 */

int main(int argc, char **argv) {
  /* Variable setup */
  char *brokers = NULL;
  char errstr[512];

  /* Signal setup */
  signal(SIGINT, signal_callback_handler);
  signal(SIGTERM, signal_callback_handler);

  /* Kafka configuration */
  conf.rk_conf = rd_kafka_conf_new();

  /* Default configuration */
  load_default_conf();

  /* Kafka Topic configuration */
  conf.topic_conf = rd_kafka_topic_conf_new();
  rd_kafka_conf_set_dr_cb(conf.rk_conf, msg_delivered);

  /* Dynamic configuration */
  char opt;
  bool dump = false;
  while ((opt = getopt(argc, argv, "p:Hhd")) != -1) {
    switch (opt) {
    case 'p':
      conf.props = strdup(optarg);
      break;

    case 'd':
      dump = true;
      break;

    case 'H':
      rd_kafka_conf_properties_show(stdout);
      exit(1);

    case 'h':
    default:
      show_usage(argv);
      exit(1);
    }
  }

  if (conf_file_read(conf.props) == -1) {
    nlc_log_error("Failed to read props file %s\n", conf.props);
    exit(EXIT_FAILURE);
  }

  if (!(encoder_function = select_encoder(conf.encoder))) {
    if (!conf.encoder) {
      nlc_log_error("Encoder option missing from props file: %s\n", conf.props);
    } else {
      nlc_log_error("Unknown encoder option: %s\n", conf.encoder);
    }
    nlc_log_error("Possible encoders: \n");
    print_encoder_names();
    exit(EXIT_FAILURE);
  }

  /* Config dump */
  if (dump) {
    dump_conf();
    exit(0);
  }
  /* Create Kafka handle */
  if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf.rk_conf, errstr,
                          sizeof(errstr)))) {
    nlc_log_error("%% Failed to create new producer: %s\n", errstr);
    exit(1);
  }

  /* Set logger */
  rd_kafka_set_logger(rk, kafka_logger);
  rd_kafka_set_log_level(rk, conf.log_level);

  /* Add brokers */
  if ((brokers != NULL) && (rd_kafka_brokers_add(rk, brokers) == 0)) {
    nlc_log_error("%% No valid brokers specified\n");
    exit(1);
  }

  /* Create topic */
  rkt = rd_kafka_topic_new(rk, conf.topic, conf.topic_conf);

  /* Prepare reusable stack buffers */
  char logline[conf.max_line_length];
  char cur_file[conf.max_log_path_size];
  char next_file[conf.max_log_path_size];

  /* Open oldest log file and accompanying progress file  */
  open_log_prog_files(&logfd, &progressfd, cur_file, sizeof(cur_file));

  unsigned int no_log_count = 0; /* Count iterations without new log data */
  unsigned long long local_line_nr = 0; /* line nr for current file */

  struct timespec backoff_sleep;

  size_t read_state = 0;

  int result = 0;
  int line_too_small = 0;

  /* Core loop */
  while (conf.run) {
    /* Read current log file*/
    if ((result =
             fgets_wrapper(logline, sizeof(logline), &read_state, logfd)) &&
        result == 1) {
      /* New log line found */
      no_log_count = 0; /* Reset no_log_count on new line */
      ++local_line_nr;

      /* Allocate and set heap version of current progress position
       * This is used in the msg_callback function in order to write
       * progress results to the correct line
       */

      fpos_t *progress_pos = (fpos_t *)malloc(sizeof(fpos_t));
      if ((fgetpos(progressfd, progress_pos)) != 0) {
        nlc_log_error("Failed to get progress pos in core loop");
        exit(EXIT_FAILURE);
      }

      /* Check if this line has been sent to kafka. */
      int linestatus = read_line_status(&progressfd);

      if (linestatus == 1) {
        nlc_log_info("Line nr %llu has already been processed\n",
                     local_line_nr);
        free(progress_pos);
        continue;
      }

      /* Do not parse part of a line if it was too large to fit in the line
       * buffer. Skip to next */
      if (line_too_small) {
        nlc_log_error("Line buffer too small for line %llu in %s. Consider "
                      "changing max.line.length config option.\n",
                      local_line_nr, cur_file);
        write_line_status(3, progress_pos, &progressfd, false);
        line_too_small = 0;
        free(progress_pos);
        continue;
      }

      /* Write current line status to 0 (aka NOT submitted) */
      if (linestatus == -1) {
        nlc_log_debug("Line number %llu write 0\n", local_line_nr);
        write_line_status(0, progress_pos, &progressfd, false);
      }

      /* Binary encode json log line*/
      struct KeyValueMsg *kvm =
          (struct KeyValueMsg *)malloc(sizeof(struct KeyValueMsg));
      *kvm = (*encoder_function)(logline, strlen(logline));
      kvm->_private = progress_pos;
      if (!valid_kvm(kvm)) {
        nlc_log_error("Encoding failed for: %s", logline);
        write_line_status(2, progress_pos, &progressfd, false);
        free_kvm(kvm);
        continue;
      }

      /* Send/Produce message. */
      send_to_kafka(rk, rkt, partition, kvm);

      /* Poll to handle delivery reports */
      rd_kafka_poll(rk, 0);

    } else {
      /* No new log line found*/

      /* Line buffer is too small" */
      if (result == -1) {
        line_too_small = 1;
        nlc_log_error("Encoding failed for: %s", logline);
        continue;
      }

      /*
       * Poll to handle delivery reports
       * This should be done before log rotate check
       */
      rd_kafka_poll(rk, 0);

      /*
       * Log rotate check
       * Conditions:
       * 1. > 1 iterations without new log data
       * 2. No messages waiting to be sent to, or acknowledged by Kafka.
       *    This is to make sure there is no progress update waiting to be
       *written.
       * 3. Check to make sure there is more than one file marked as unprocessed
       *       (The current file is still marked as unprocessed, hence > 1
       *
       */
      if ((++no_log_count > 1) && (rd_kafka_outq_len(rk) == 0) &&
          (check_rotate(next_file, sizeof(next_file), conf.watch_dir) > 1)) {
        nlc_log_info("Done processing %s, ", cur_file);

        fflush(NULL);

        rename_and_close_files(&logfd, &progressfd, cur_file);

        open_log_prog_files(&logfd, &progressfd, cur_file, sizeof(cur_file));
        local_line_nr = 0; /* Reset line nr count for new file */
        sleep(1);          /* sleep 1 before processing next logfile */
      } else {
        clearerr(logfd); // clear EOF
        fflush(NULL);    // Flush data do disk while waiting

        /* linear sleep throttle */
        unsigned long pt = conf.incr_backoff * no_log_count;
        unsigned long ms = (pt > conf.max_backoff) ? conf.max_backoff : pt;
        ms2ts(&backoff_sleep, ms);
        nlc_log_info("No new logs found, sleeping for %lu ms\n", ms);
        nanosleep(&backoff_sleep, NULL);
      }
    }
  }

  // Wait for all currently queued messages to be sent
  signal(SIGINT, interrupt_callback_handler);
  while (rd_kafka_outq_len(rk) != 0) {
    nlc_log_error("Waiting for all queued kafka messages to be sent\n");
    rd_kafka_poll(rk, 500);
    fflush(NULL);
    sleep(1);
  }
  rd_kafka_poll(rk, 500);
  return 0;
}
