#define _GNU_SOURCE
#define SORT versionsort
#if __APPLE__
#undef SORT
#define SORT alphasort
#endif
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "encoders/encoders.h"
#include "log_api.h"
#define UTILS_LOG_BUF_SIZE 1024

/*
 * Converts unsigned long representing milliseconds
 * to timespec format.
 *
 */
void ms2timespec(struct timespec* ts, unsigned long ms) {
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

/*
 * Blocking version of looking for new file to open
 * This is usefull during startup in case lts is started before any log file
 *exists.
 *
 * return: 1 on sucess, -1 on failure
 */

static int get_oldest_filename(char* buf, size_t buf_size, const char* dir_path) {
  int exist = 0;
  do {
    exist = search_oldest_uncomplete(buf, buf_size, dir_path);
    if (!exist) sleep(2);
  } while ((!exist) && conf.run);
  if (!conf.run) return -1;
  return 1;
}

/*
 * Send data to kafka
 * This function will block and retry the librdkafka produce api.
 * This is to handle possible produce errors from librdkafka.
 * This function also polls msg_callback to still process callbacks.
 *
 * More information in rdkafka.h (librdkafka api document)
 * Input:
 *  msg: pointer to data to be sent
 *  msg_size: data size
 *  msg_opaque: pointer to data that will be returned during msg_callback
 *
 *
 * return: 0 on sucess, -1 on failure
 */

int send_to_server(rd_kafka_t* rk, rd_kafka_topic_t* rkt, int32_t partition,
                  struct KeyValueMsg* kvm) {
  char log_buf[UTILS_LOG_BUF_SIZE] = {0};
  int ret = -1;
#define RD_KAFKA_MSG_F_DO_NOTHING 0
  while (
      (ret = rd_kafka_produce(rkt, partition, RD_KAFKA_MSG_F_DO_NOTHING, kvm->value,
                                 kvm->value_size, kvm->key, kvm->key_size, (void*) kvm) == -1) &&
      conf.run) 
	{
    rd_kafka_resp_err_t kafka_error = rd_kafka_errno2err(errno);
   // lts_log_error("Failed to produce to topic %s partition %i: %s\n",
    //        rd_kafka_topic_name(rkt), partition, rd_kafka_err2str(kafka_error));
    snprintf(log_buf, UTILS_LOG_BUF_SIZE, "Failed to produce to topic %s partition %i: %s", rd_kafka_topic_name(rkt), partition, rd_kafka_err2str(kafka_error));
		lts_public_log_error(log_buf);
		switch (kafka_error) {
      case RD_KAFKA_RESP_ERR__QUEUE_FULL:
        break;
      case RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE:
        //lts_log_error("MSG_SIZE_TOO_LARGE\n");
				lts_public_log_error("MSG_SIZE_TOO_LARGE!");
        exit(EXIT_FAILURE);
      case RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION:
        //lts_log_error("UNKNOWN_PARTITION\n");
				lts_public_log_error("UNKNOWN_PARTITION!");
        exit(EXIT_FAILURE);
      case RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC:
        //lts_log_error("UNKNOWN TOPIC\n");
				lts_public_log_error("UNKNOWN TOPIC!");
        exit(EXIT_FAILURE);
      default:
        break;
    }

    /* Blocking poll to handle delivery reports */
    rd_kafka_poll(rk, 2000);
  }
  return ret;
}

/*
 * Read status from a line in a progress file
 *
 * return: status number on sucess. -1 on failure.
 */

long int fgets_line_status(FILE** progfd) {
  long int result = -1;
  char data[512];

  if (fgets(data, sizeof(data), *progfd) != NULL) {
    errno =
        0; /* To distinguish success/failure after call. necessary for strtol*/
    char* endptr;
    result = strtol(data, &endptr, 0);
    if ((errno == ERANGE && (result == LONG_MAX || result == LONG_MIN)) ||
        (errno != 0 && result == 0) || (endptr == data)) {
      //lts_log_error("Error parsing processed status");
      lts_public_log_error("Error parsing processed status!");
      result = -1;
    }
  }
  return result;
}

/*
 * Write line status to a progress file
 *
 * Input:
 *  status: status number (0,1) (unprocessed,processed)
 *  write_pos: output position
 *  progfd: progress file
 *
 * return: status number on sucess. 0 on failure (equal to not processed status)
 */

void fputs_line_status(int status, fpos_t* write_pos, FILE** progfd,
                       bool resetpos) {
  char send_status[3];  // 0|1 + \n + \0 . snprintf always adds null char
  fpos_t cur_position;

  if (snprintf(send_status, sizeof(send_status), "%i\n", status) < 0) {
    //lts_log_error("Failed to create new name string\n");
		lts_public_log_error("Failed to create new name string!");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if (fgetpos(*progfd, &cur_position) != 0) {
    //lts_log_error("Failed to get file position of progress file");
		lts_public_log_error("Failed to get file position of progress file!");
    exit(EXIT_FAILURE);
  }

  if (fsetpos(*progfd, write_pos) != 0) {
    //lts_log_error("Failed to change position of progress file");
		lts_public_log_error("Failed to change position of progress file!");
    exit(EXIT_FAILURE);
  }

  if (fputs(send_status, *progfd) < 0) {
    //lts_log_error("Failed to write to progress file");
		lts_public_log_error("Failed to write to progress file!");
    exit(EXIT_FAILURE);
  }

  if (resetpos) {
    if (fsetpos(*progfd, &cur_position) != 0) {
      //lts_log_error("Failed to change position of progress file");
			lts_public_log_error("Failed to change position of progress file!");
      exit(EXIT_FAILURE);
    }
  }
}

/*
 * Open log and progress file this will block if there is no new file to open.
 *
 *
 */

void open_oldest_files(FILE** logfd, FILE** progfd, char* file_name,
                         size_t filename_size) {
  char progress_file[conf.max_path_size];

  get_oldest_filename(file_name, filename_size, conf.watch_dir);

  /* Open logfile file*/
  if ((*logfd = fopen(file_name, "r")) == NULL) {
    //lts_log_error("Error opening file\n");
		lts_public_log_error("Error opening file!");
    exit(EXIT_FAILURE);
  }

  /* Open progress file*/
  if (snprintf(progress_file, sizeof(progress_file), "%s%s", file_name, conf.prog_ext) <
      0) {
    //lts_log_error("Failed to create new name string\n");
		lts_public_log_error("Failed to create new name string!");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if ((*progfd = fopen(progress_file, "a+")) == NULL) {
    //lts_log_error("Failed to open progress file");
    lts_public_log_error("Failed to open progress file!");
    exit(EXIT_FAILURE);
  }

  if ((*progfd = freopen(progress_file, "r+", *progfd)) == NULL) {
    //lts_log_error("Failed to freopen progress file");
		lts_public_log_error("Failed to freopen progress file!");
    exit(EXIT_FAILURE);
  }
}

/*
 * Rename file to complete ext and close file descriptors
 *
 *
 */

void rename_and_close_files(FILE** logfd, FILE** progfd, char* cur_file) {
  char complete_file[conf.max_path_size];

  if (snprintf(complete_file, conf.max_path_size, "%s%s", cur_file, conf.comp_ext) <
      0) {
    //lts_log_error("Failed to create new name string\n");
		lts_public_log_error("Failed to create new name string!");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if (rename(cur_file, complete_file) != 0) {
    //lts_log_error("Failed to rename log file");
		lts_public_log_error("Failed to rename log file!");
    exit(EXIT_FAILURE);
  }

  if (fclose(*logfd) != 0) {
    //lts_log_error("Failed to close log file");
    lts_public_log_error("Failed to close log file!");
    exit(EXIT_FAILURE);
  }

  if (fclose(*progfd) != 0) {
    //lts_log_error("Failed to close progress file file");
		lts_public_log_error("Failed to close progress file file!");
    exit(EXIT_FAILURE);
  }
}

/*
 *	fgets but working on file pointer from open().
 *	caller provides a buffer of size size.
 */

/*
char* rgets(char* buf, size_t size, int fp)
{
    size_t read_size;
    if (((read_size = read(fp, buf, size)) <= 0)) return NULL;
    char* pch;
    pch = strchr(buf, '\n');
    if (pch == NULL) {
        lseek(fp, -read_size, SEEK_CUR);
        return NULL;
    }
    off_t offset = -(read_size - ((pch + 1) - buf)); // (pch + 1) == skip \n
    *pch = '\0';
    lseek(fp, offset, SEEK_CUR);
    return buf;
}
*/

/*
 *	Filter out files that has already been processed
 *	as indicated by the comp_ext string.
 */

static int log_filter(const struct dirent* entry) {
  char* complete_flag = strstr(entry->d_name, conf.comp_ext);
  char* progress_flag = strstr(entry->d_name, conf.prog_ext);
  int len = strlen(entry->d_name); /* remove . and .. */
  return ((len > 2) && (complete_flag ? 0 : 1) && (progress_flag ? 0 : 1));
}

/*
 *	Return the filename of the oldest, non-processed file. If none return
 *NULL.
 *	Oldest in this case is done by sorting the filenames using version sort.
 *
 *	Memory:
 *	The buffer should have enough space to fit NAME_MAX + 1 (nullchar) and
 *the dir path.
 *
 *   Returns:
 *   The number of files found, or < 0  on error
 */

int search_oldest_uncomplete(char* buf, size_t buf_size, const char* dir_path) {
  struct dirent** name_array;
  int n, file_num = 0, ret = 0;
  char* file_path = buf;

//  lts_log_info("Scanning %s \n", dir_path);

  file_num = scandir(dir_path, &name_array, log_filter, SORT);
  n = file_num;
  if (n < 0)
  {
  	//lts_log_error("Failed to scan dir");
		lts_public_log_error("search_oldest_uncomplete,Failed to scan dir!");
  }
  else if (n > 0) {
    ret =
        snprintf(file_path, buf_size, "%s%s", dir_path, name_array[0]->d_name);
    while (n--) {
      free(name_array[n]);
    }
    free(name_array);
  }
  if (ret < 0) file_num = ret;
  return file_num;
}

/*
*  fgets_wrapper uses fgets to read until \n
*  In order to handle partial written files the caller has to provide a int*
*read_state
*  that is used to manage how much has been read of a line so far for each call.
*  read_state should be initialized to 0 before first call and then never
*changed externally.
*
*   return: 1 => successfully read a line, line content is stored in *dst
*           0 => partially read line, read_state has been uptaded for next call
*          -1 => Input buffer is too small, read_state has been reset to 0 and
*part of line will be lost.
*          -2 => No new data on buffer
*     -1 and -2 could be caused by EOF. To continue read from the same file the
*EOF flag has to be cleared.
*/
int fgets_file_data(char* dst, size_t max, size_t* position, FILE* fp) {
  int ret = -2;
  char* fgetsresult = fgets((dst + *position), (max - *position), fp);
  if (fgetsresult) {
    size_t len = strlen(dst);
    if (*((dst + len) - 1) == '\n') {
      *position = 0;
      ret = 1;
    } else if (len + 1 == max) {
      *position = 0;
      ret = -1;
    } else {
      *position = len;
      ret = 0;
    }
  }
  return ret;
}
