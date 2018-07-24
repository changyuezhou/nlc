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

/*
 * Converts unsigned long representing milliseconds
 * to timespec format.
 *
 */
void ms2ts(struct timespec* ts, unsigned long ms) {
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
}

/*
 * Blocking version of looking for new file to open
 * This is usefull during startup in case nlc is started before any log file
 *exists.
 *
 * return: 1 on sucess, -1 on failure
 */

static int get_file_block(char* buf, size_t buf_size, const char* dir_path) {
  int sucess = 0;
  do {
    sucess = check_rotate(buf, buf_size, dir_path);
    if (!sucess) sleep(2);
  } while ((!sucess) && conf.run);
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

int send_to_kafka(rd_kafka_t* rk, rd_kafka_topic_t* rkt, int32_t partition,
                  struct KeyValueMsg* kvm) {
  int result = -1;
#define RD_KAFKA_MSG_F_DO_NOTHING 0
  while (
      (result = rd_kafka_produce(rkt, partition, RD_KAFKA_MSG_F_DO_NOTHING, kvm->value,
                                 kvm->value_size, kvm->key, kvm->key_size, (void*) kvm) == -1) &&
      conf.run) {
    rd_kafka_resp_err_t kafka_error = rd_kafka_errno2err(errno);
    nlc_log_error("Failed to produce to topic %s partition %i: %s\n",
            rd_kafka_topic_name(rkt), partition, rd_kafka_err2str(kafka_error));
    switch (kafka_error) {
      case RD_KAFKA_RESP_ERR__QUEUE_FULL:
        break;
      case RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE:
        nlc_log_error("MSG_SIZE_TOO_LARGE\n");
        exit(EXIT_FAILURE);
      case RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION:
        nlc_log_error("UNKNOWN_PARTITION\n");
        exit(EXIT_FAILURE);
      case RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC:
        nlc_log_error("UNKNOWN TOPIC\n");
        exit(EXIT_FAILURE);
      default:
        break;
    }

    /* Blocking poll to handle delivery reports */
    rd_kafka_poll(rk, 2000);
  }
  return result;
}

/*
 * Read status from a line in a progress file
 *
 * return: status number on sucess. -1 on failure.
 */

long int read_line_status(FILE** progfd) {
  long int processed = -1;
  char prog_line[512];

  if (fgets(prog_line, sizeof(prog_line), *progfd) != NULL) {
    errno =
        0; /* To distinguish success/failure after call. necessary for strtol*/
    char* endptr;
    processed = strtol(prog_line, &endptr, 0);
    if ((errno == ERANGE && (processed == LONG_MAX || processed == LONG_MIN)) ||
        (errno != 0 && processed == 0) || (endptr == prog_line)) {
      nlc_log_error("Error parsing processed status");
      processed = -1;
    }
  }
  return processed;
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

void write_line_status(int status, fpos_t* write_pos, FILE** progfd,
                       bool resetpos) {
  char prog_line[3];  // 0|1 + \n + \0 . snprintf always adds null char
  fpos_t cur_pos;

  if (snprintf(prog_line, sizeof(prog_line), "%i\n", status) < 0) {
    nlc_log_error("Failed to create new name string\n");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if (fgetpos(*progfd, &cur_pos) != 0) {
    nlc_log_error("Failed to get file position of progress file");
    exit(EXIT_FAILURE);
  }

  if (fsetpos(*progfd, write_pos) != 0) {
    nlc_log_error("Failed to change position of progress file");
    exit(EXIT_FAILURE);
  }

  if (fputs(prog_line, *progfd) < 0) {
    nlc_log_error("Failed to write to progress file");
    exit(EXIT_FAILURE);
  }

  if (resetpos) {
    if (fsetpos(*progfd, &cur_pos) != 0) {
      nlc_log_error("Failed to change position of progress file");
      exit(EXIT_FAILURE);
    }
  }
}

/*
 * Open log and progress file this will block if there is no new file to open.
 *
 *
 */

void open_log_prog_files(FILE** logfd, FILE** progfd, char* cur_file,
                         size_t cur_file_size) {
  char prog_file[conf.max_path_size];

  get_file_block(cur_file, cur_file_size, conf.watch_dir);

  /* Open logfile file*/
  if ((*logfd = fopen(cur_file, "r")) == NULL) {
    nlc_log_error("Error opening file\n");
    exit(EXIT_FAILURE);
  }

  /* Open progress file*/
  if (snprintf(prog_file, sizeof(prog_file), "%s%s", cur_file, conf.prog_ext) <
      0) {
    nlc_log_error("Failed to create new name string\n");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if ((*progfd = fopen(prog_file, "a+")) == NULL) {
    nlc_log_error("Failed to open progress file");
    exit(EXIT_FAILURE);
  }

  if ((*progfd = freopen(prog_file, "r+", *progfd)) == NULL) {
    nlc_log_error("Failed to freopen progress file");
    exit(EXIT_FAILURE);
  }
}

/*
 * Rename file to complete ext and close file descriptors
 *
 *
 */

void rename_and_close_files(FILE** logfd, FILE** progfd, char* cur_file) {
  char comp_file[conf.max_path_size];

  if (snprintf(comp_file, conf.max_path_size, "%s%s", cur_file, conf.comp_ext) <
      0) {
    nlc_log_error("Failed to create new name string\n");
    exit(EXIT_FAILURE);  // Could this be abused?
  }

  if (rename(cur_file, comp_file) != 0) {
    nlc_log_error("Failed to rename log file");
    exit(EXIT_FAILURE);
  }

  if (fclose(*logfd) != 0) {
    nlc_log_error("Failed to close log file");
    exit(EXIT_FAILURE);
  }

  if (fclose(*progfd) != 0) {
    nlc_log_error("Failed to close progress file file");
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
  char* result_comp = strstr(entry->d_name, conf.comp_ext);
  char* result_prog = strstr(entry->d_name, conf.prog_ext);
  int length = strlen(entry->d_name); /* remove . and .. */
  return ((length > 2) && (result_comp ? 0 : 1) && (result_prog ? 0 : 1));
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

int check_rotate(char* buf, size_t buf_size, const char* dir_path) {
  struct dirent** namelist;
  int n, scandir_res = 0, snprint_res = 0;
  char* filepath = buf;

  nlc_log_info("Scanning %s \n", dir_path);

  scandir_res = scandir(dir_path, &namelist, log_filter, SORT);
  n = scandir_res;
  if (n < 0)
    nlc_log_error("Failed to scan dir");
  else if (n > 0) {
    snprint_res =
        snprintf(filepath, buf_size, "%s%s", dir_path, namelist[0]->d_name);
    while (n--) {
      free(namelist[n]);
    }
    free(namelist);
  }
  if (snprint_res < 0) scandir_res = snprint_res;
  return scandir_res;
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
int fgets_wrapper(char* dst, size_t max, size_t* read_state, FILE* fp) {
  int result = -2;
  char* fgetsresult = fgets((dst + *read_state), (max - *read_state), fp);
  if (fgetsresult) {
    size_t len = strlen(dst);
    if (*((dst + len) - 1) == '\n') {
      *read_state = 0;
      result = 1;
    } else if (len + 1 == max) {
      *read_state = 0;
      result = -1;
    } else {
      *read_state = len;
      result = 0;
    }
  }
  return result;
}
