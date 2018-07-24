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
#include "lts.h"
#include "utils.h"
#include "common_struct.h"
#include "config_api.h"
#include "version_api.h"
#include "log_api.h"

#ifndef VERSION
#define VERSION "no-verson-info"
#endif
#ifndef TAG
#define TAG "no-tag-info"
#endif
#ifndef BUILD_TIME
#define BUILD_TIME "no-build-time-info"
#endif

#define LTS_LOG_BUF_SIZE   1024

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


void print_config_info()
{
	printf("# lts config\n");
	printf("topic = %s\n", conf.topic);
	printf("watch.dir = %s\n", conf.watch_dir);
	printf("log.level = %i\n", conf.log_level);
	printf("encoder = %s\n", conf.encoder);
	printf("max.line.length = %lu\n", conf.max_line_length);
	printf("proto.type.name = %s\n", conf.proto_type_name);
	printf("proto.type.folder = %s\n", conf.proto_type_folder);
	printf("proto.type.key.name = %s\n", conf.proto_key_type_name);

}

//bruce add
int re_init_config()
{

	char log_buf[LTS_LOG_BUF_SIZE] = {0};
	
	get_nanosleep(&conf.incr_backoff, &conf.max_backoff);
	conf.watch_dir = get_watchdir();
	conf.watch_dir_len = strlen(conf.watch_dir);

	proto_info_c pbinfo; 
	get_protobuf_info(&pbinfo);
	conf.proto_type_name = pbinfo.type_name;
	conf.proto_type_folder = pbinfo.type_folder;
	//conf.proto_key_type_name = pbinfo.key_name;
	conf.proto_key_type_name = NULL;

	//
	other_c othe;
	get_other_info(&othe);
	conf.encoder = othe.encoder;
	conf.max_line_length = othe.max_line_len;

	int number = get_kafka_config_number();
	config_info_c data;
	char errstr[512];
	int errstr_size = sizeof(errstr);

	int i;
	for (i=0; i<number; i++)
	{
		get_kafka_index_config( i, &data);

		if (!strcmp(data.name, "partitioner")) 
		{
		    if (strcmp(data.value, "random") == 0) 
			{
		    	rd_kafka_topic_conf_set_partitioner_cb(conf.topic_conf, rd_kafka_msg_partitioner_random);
				continue;
		    }
			else if (strcmp( data.value, "consistent") == 0) 
			{
		    	rd_kafka_topic_conf_set_partitioner_cb(conf.topic_conf, rd_kafka_msg_partitioner_consistent);
				continue;
		    } 
			else 
			{
		    	return -1;
		    }
  		}

		if (!strcmp(data.name, "log.level")) {
			if (!strcasecmp(data.value, "debug")) 
			{
				conf.log_level = NLC_DEBUG;
				continue;
			}
			else if (!strcasecmp(data.value, "info")) 
			{
				conf.log_level = NLC_INFO;
				continue;
			} 
			else if (!strcasecmp( data.value, "error")) 
			{
				conf.log_level = NLC_ERROR;
				continue;
			} 
			else 
			{
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "log_level config failed!");
				lts_public_log_error(log_buf);
				return -1;
			}
		}

		if (!strncmp(data.name, "kafka.", strlen("kafka."))) 
		{
			const char *kafka_name = data.name + strlen("kafka.");
			rd_kafka_conf_res_t res;
			res = RD_KAFKA_CONF_UNKNOWN;

			/* Kafka topic configuration. */
			if (!strncmp(kafka_name, "topic.", strlen("topic.")))
			{
				res = rd_kafka_topic_conf_set(conf.topic_conf, kafka_name + strlen("topic."), data.value, errstr, errstr_size);
			}
			else /* Kafka global configuration */
			{
				res =rd_kafka_conf_set(conf.rk_conf, kafka_name, data.value, errstr, errstr_size);
			}

			if (res == RD_KAFKA_CONF_OK)
			{
				//printf("Added kafka config option %s=%s\n", data.name, data.value);
				continue;
			}
			else
			{
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "kafka  config set %s failed!", data.name);
				lts_public_log_error(log_buf);
				return -1;
			}
		}

		if (!strcmp(data.name, "topic")) 
		{
		    conf.topic = strdup(data.value);
  		}
				
	}

	return 0;
}




void interrupt_callback_handler(int signum) {
	lts_public_log_error("Second interrup signal, hard shutdown!");
  //lts_log_error("Second interrup signal, hard shutdown!");
  exit(0);
}

void signal_callback_handler(int signum) {
  char log_buf[LTS_LOG_BUF_SIZE] = {0};
	snprintf(log_buf, LTS_LOG_BUF_SIZE, "Graceful shutdown started, Caught signal %d", signum);
	lts_public_log_error(log_buf);
  conf.run = 0;
}


static void display_helper_info(char **argv) {
  lts_log_error("Usage: %s -p <file.prop>"
                "\n"
                "librdkafka version %s (0x%08x)\n"
                "lts version %s (tag: %s)\n"
                "build time: %s\n"
                "\n"
                " Options:\n"
                "  -p <file.prop>  lts properties file (default: lts.props)\n"
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
  struct timeval tv_tmp;
  gettimeofday(&tv_tmp, NULL);
  //fprintf(stdout, "%u.%03u RDKAFKA-%i-%s: %s: %s\n", (int)tv_tmp.tv_sec, (int)(tv_tmp.tv_usec / 1000), level, fac, rd_kafka_name(rk), buf);
  
  char log_buf[LTS_LOG_BUF_SIZE] = {0};
	snprintf(log_buf, LTS_LOG_BUF_SIZE, "%u.%03u RDKAFKA-%i-%s: %s: %s\n", (int)tv_tmp.tv_sec,(int)(tv_tmp.tv_usec / 1000), level, fac, rd_kafka_name(rk), buf);
	lts_public_log_warn(log_buf);
	
}

/*
 * Message delivery report callback.
 * Called once for each message.
 * This function is responsible for freeing memory.
 */

static void msg_delivered(rd_kafka_t *rk, void *payload, size_t len,
                          int error_code, void *opaque, void *msg_opaque) {

  //printf("msg_delivered error_code=%d\n", error_code);
  struct KeyValueMsg *kvm = (struct KeyValueMsg *)msg_opaque;
  if (error_code) {
		char log_buf[LTS_LOG_BUF_SIZE] = {0};
		snprintf(log_buf, LTS_LOG_BUF_SIZE, "%% Message delivery failed: %s\n Readding to kafka queue\n", rd_kafka_err2str(error_code));
		lts_public_log_warn(log_buf);
    //lts_log_error("%% Message delivery failed: %s\n Readding to kafka queue\n",
    //              rd_kafka_err2str(error_code));
    send_to_server(rk, rkt, partition, kvm);
  } else {
    fpos_t *pos = (fpos_t *)kvm->_private;
    /* Write current line status to 1 (aka submitted) */
    fputs_line_status(1, pos, &progressfd, true);
    ++global_line_count;
    free_kvm(kvm);
  }
}


/*
 * Main function entry
 */
//logline
int main(int argc, char **argv) {
  /* Variable setup */
  char *brokers_tmp = NULL;
  char errstr_tmp[512];
  char ver[1024] = {0};


  get_compile_time(ver);
 // printf("ver=%s\n", ver);
 
  /* Signal setup */
  signal(SIGINT, signal_callback_handler);
  signal(SIGTERM, signal_callback_handler);

  /* Kafka configuration */
  conf.rk_conf = rd_kafka_conf_new();

  /* Default configuration */
  init_default_config();

  /* Kafka Topic configuration */
  conf.topic_conf = rd_kafka_topic_conf_new();
  rd_kafka_conf_set_dr_cb(conf.rk_conf, msg_delivered);

  /* Dynamic configuration */
  char option;
  bool dump_flag = false;
  while ((option = getopt(argc, argv, "p:Hhd")) != -1) {
    switch (option) {
    case 'p':
      conf.props = strdup(optarg);
      break;

    case 'd':
      dump_flag = true;
      break;

    case 'H':
      rd_kafka_conf_properties_show(stdout);
      exit(1);

    case 'h':
    default:
      display_helper_info(argv);
      exit(1);
    }
  }

	if (parse_config_file(conf.props) == -1)
	{
		lts_public_log_error("Failed to read props file!");
    exit(EXIT_FAILURE);
	}
	
	if (re_init_config() == -1)
	{
		lts_public_log_error("re_init_config error!\n");
    exit(EXIT_FAILURE);
	}

	int result = parser_log_config(conf.props);

//	print_config_info();
  if (!(encoder_function = init_encoder_by_type(conf.encoder))) {
    if (!conf.encoder) {
      lts_public_log_error("Encoder option missing from config file!");
    } else {
      lts_public_log_error("Unknown encoder option!");
    }
    lts_log_error("Possible encoders: \n");
    print_encoder_names();
    exit(EXIT_FAILURE);
  }

  /* Config dump */
  if (dump_flag) {
    display_pb_cofig();
    exit(0);
  }
  /* Create Kafka handle */
  if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf.rk_conf, errstr_tmp,
                          sizeof(errstr_tmp)))) {
    lts_public_log_error("%% Failed to create new producer!");
    exit(1);
  }

  /* Set logger */
  rd_kafka_set_logger(rk, kafka_logger);
  rd_kafka_set_log_level(rk, conf.log_level);

  /* Add brokers */
  if ((brokers_tmp != NULL) && (rd_kafka_brokers_add(rk, brokers_tmp) == 0)) {
    lts_public_log_error("%% No valid brokers specified\n");
    exit(1);
  }

  /* Create topic */
  rkt = rd_kafka_topic_new(rk, conf.topic, conf.topic_conf);

  /* Prepare reusable stack buffers */
  char log_data[conf.max_line_length];
  char cur_log_file[conf.max_log_path_size];
  char next_log_file[conf.max_log_path_size];


  /* Open oldest log file and accompanying progress file  */
  open_oldest_files(&logfd, &progressfd, cur_log_file, sizeof(cur_log_file));

  unsigned int empty_count = 0; /* Count iterations without new log data */
  unsigned long long file_line_number = 0; /* line nr for current file */

  struct timespec nano_sleep;

  size_t positon = 0;

  int ret = 0;
  int big_data = 0;
	char log_buf[LTS_LOG_BUF_SIZE] = {0};

  /* Core loop */
  while (conf.run) 
  {
  	memset(log_buf, 0, LTS_LOG_BUF_SIZE);
    /* Read current log file*/
    if ((ret = fgets_file_data(log_data, sizeof(log_data), &positon, logfd)) && ret == 1) 
		{

      /* New log line found */
      empty_count = 0; /* Reset no_log_count on new line */
      ++file_line_number;

      /* Allocate and set heap version of current progress position
	       * This is used in the msg_callback function in order to write
	       * progress results to the correct line
	       */
      fpos_t *progress_pos = (fpos_t *)malloc(sizeof(fpos_t));
      if ((fgetpos(progressfd, progress_pos)) != 0) 
	  	{
        lts_public_log_error("Failed to get progress pos in core loop!");
        exit(EXIT_FAILURE);
      }

      /* Check if this line has been sent to kafka. */
      int send_status = fgets_line_status(&progressfd);

      if (send_status == 1) 
	  	{
	  		snprintf(log_buf, LTS_LOG_BUF_SIZE, "Line nr %llu has already been processed\n", file_line_number);
				lts_public_log_debug(log_buf);
        //lts_public_log_info("Line nr %llu has already been processed\n", file_line_number);
        free(progress_pos);
        continue;
      }

      /* Do not parse part of a line if it was too large to fit in the line
           * buffer. Skip to next */
      if (big_data) 
	  	{
	  		snprintf(log_buf, LTS_LOG_BUF_SIZE, "Line buffer too small for line %llu in %s. Consider changing max.line.length config option.", file_line_number, cur_log_file);
				lts_public_log_error(log_buf);
				//lts_public_log_error("Line buffer too small for line %llu in %s. Consider "
        //	      "changing max.line.length config option.\n",
        //              file_line_number, cur_log_file);
        
        fputs_line_status(3, progress_pos, &progressfd, false);
        big_data = 0;
        free(progress_pos);
        continue;
      }

      /* Write current line status to 0 (aka NOT submitted) */
      if (send_status == -1) 
	  	{
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "Line number %llu write 0\n", file_line_number);
				lts_public_log_debug(log_buf);
        //lts_public_log_debug("Line number %llu write 0\n", file_line_number);
        fputs_line_status(0, progress_pos, &progressfd, false);
      }

      /* Binary encode json log line*/
      struct KeyValueMsg *kvm = (struct KeyValueMsg *)malloc(sizeof(struct KeyValueMsg));
      *kvm = (*encoder_function)(log_data, strlen(log_data));
      kvm->_private = progress_pos;
      if (!judge_kvm(kvm)) 
	  	{
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "Encoding failed for: %s", log_data);
				lts_public_log_error(log_buf);
				//lts_public_log_error("Encoding failed for: %s", log_data);
				fputs_line_status(2, progress_pos, &progressfd, false);
        free_kvm(kvm);
        continue;
      }

      /* Send/Produce message. */
      send_to_server(rk, rkt, partition, kvm);

      /* Poll to handle delivery reports */
      rd_kafka_poll(rk, 0);

    } 
		else 
		{
      /* No new log line found*/

      /* Line buffer is too small" */
      if (ret == -1) {
        big_data = 1;
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "Encoding failed for: %s", log_data);
				lts_public_log_error(log_buf);
				//lts_public_log_error("Encoding failed for: %s", log_data);

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
      if ((++empty_count > 1) && (rd_kafka_outq_len(rk) == 0) &&
          (search_oldest_uncomplete(next_log_file, sizeof(next_log_file), conf.watch_dir) > 1)) 
      {
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "Done processing %s, ", cur_log_file);
				lts_public_log_info(log_buf);

        fflush(NULL);
        rename_and_close_files(&logfd, &progressfd, cur_log_file);
        open_oldest_files(&logfd, &progressfd, cur_log_file, sizeof(cur_log_file));
        file_line_number = 0; /* Reset line nr count for new file */
        sleep(1);          /* sleep 1 before processing next logfile */
      }
		  else 
		  {
        clearerr(logfd); // clear EOF
        fflush(NULL);    // Flush data do disk while waiting

        /* linear sleep throttle */
        unsigned long pt = conf.incr_backoff * empty_count;
        unsigned long ms = (pt > conf.max_backoff) ? conf.max_backoff : pt;
        ms2timespec(&nano_sleep, ms);
				
				snprintf(log_buf, LTS_LOG_BUF_SIZE, "Done processing %s, ", cur_log_file);
				lts_public_log_info(log_buf);
        //lts_public_log_info("No new logs found, sleeping for %lu ms\n", ms);
        nanosleep(&nano_sleep, NULL);
      }
    }
  }

  // Wait for all currently queued messages to be sent
  signal(SIGINT, interrupt_callback_handler);
  while (rd_kafka_outq_len(rk) != 0) {
    lts_log_error("Waiting for all queued kafka messages to be sent\n");
    rd_kafka_poll(rk, 500);
    fflush(NULL);
    sleep(1);
  }
  rd_kafka_poll(rk, 500);
  return 0;
}
