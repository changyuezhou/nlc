#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <syslog.h>

#include "lts.h"

/**
 * lts global configuration
 */
struct conf conf = { 0 };



void init_default_config() {
    /* Static configuration */
  conf.encoder = NULL;
  conf.topic = "";
  conf.props = "lts.conf";
  conf.comp_ext = ".COMPLETE";
  conf.comp_ext_len = strlen(conf.comp_ext);
  conf.prog_ext = ".PROGRESS";
  conf.prog_ext_len = strlen(conf.prog_ext);
  conf.watch_dir = "watch_dir/";
  conf.watch_dir_len = strlen("watch_dir/");
  conf.log_level = NLC_INFO;
  conf.max_log_path_size =
      NAME_MAX - EXT_RESERVE + conf.watch_dir_len + 1;     // +1 = null char
  conf.max_path_size = NAME_MAX + conf.watch_dir_len + 1;  // +1 = null char
  conf.run = 1;
  conf.max_line_length = 2048;
  conf.max_backoff = 30000;
  conf.incr_backoff = 2000;
  conf.proto_key_type_name = NULL;
}

void display_pb_cofig() {
  const char **array;
  size_t cnt;
  int pass;

  for (pass = 0; pass < 2; pass++) {
    size_t i;

    if (pass == 0) {
      array = rd_kafka_conf_dump(conf.rk_conf, &cnt);
      printf("# Global kafka config\n");
    } else {
      printf("# Topic kafka config\n");
      array = rd_kafka_topic_conf_dump(conf.topic_conf, &cnt);
    }

    for (i = 0; i < cnt; i += 2) printf("kafka.%s = %s\n", array[i], array[i + 1]);

    printf("\n");

    rd_kafka_conf_dump_free(array, cnt);
  }
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

/**
 * Set a single configuration property 'name' using value 'val'.
 * Returns 0 on success, and -1 on error in which case 'errstr' will
 * contain an error string.
 */
static int conf_set(const char *name, const char *val, char *errstr,
                    size_t errstr_size) {
  int ok = 0;
  /* Kafka configuration */
  if (!strncmp(name, "kafka.", strlen("kafka."))) {
    const char *kafka_name = name + strlen("kafka.");
    rd_kafka_conf_res_t res;
    res = RD_KAFKA_CONF_UNKNOWN;

    /* Kafka topic configuration. */
    if (!strncmp(kafka_name, "topic.", strlen("topic.")))
      res = rd_kafka_topic_conf_set(conf.topic_conf,
                                    kafka_name + strlen("topic."), val, errstr,
                                    errstr_size);
    else /* Kafka global configuration */
      res =
          rd_kafka_conf_set(conf.rk_conf, kafka_name, val, errstr, errstr_size);

    if (res == RD_KAFKA_CONF_OK) {
      printf("Added kafka config option %s=%s\n", name, val);
      return 0;
    }
  } else if (!strcmp(name, "topic")) {
    conf.topic = strdup(val);
    ok = 1;
  } else if (!strcmp(name, "watch.dir")) {
    conf.watch_dir = strdup(val);
    conf.watch_dir_len = strlen(val);
    ok = 1;
  } else if (!strcmp(name, "encoder")) {
    conf.encoder = strdup(val);
    ok = 1;
  } else if (!strcmp(name, "log.level")) {
    if (!strcasecmp(val, "debug")) {
      conf.log_level = NLC_DEBUG;
      ok = 1;
    } else if (!strcasecmp(val, "info")) {
      conf.log_level = NLC_INFO;
      ok = 1;
    } else if (!strcasecmp(val, "error")) {
      conf.log_level = NLC_ERROR;
      ok = 1;
    } else {
      snprintf(errstr, errstr_size, "Unkown log level '%s', (valid: debug,info,error)", val);
    }
  } else if (!strcmp(name, "max.line.length")) {
    conf.max_line_length = strtoul(val, NULL, 0);
    ok = 1;
  } else if (!strcmp(name, "proto.type.name")) {
    conf.proto_type_name = strdup(val);
    ok = 1;
  } else if (!strcmp(name, "proto.type.folder")) {
    conf.proto_type_folder = strdup(val);
    ok = 1;
  } else if (!strcmp(name, "backoff.max.ms")) {
    conf.max_backoff = strtoul(val, NULL, 0);
    ok = 1;
  } else if (!strcmp(name, "backoff.incr.ms")) {
    conf.incr_backoff = strtoul(val, NULL, 0);
    ok = 1;
  } else if (!strcmp(name, "proto.type.key.name")) {
    conf.proto_key_type_name = strdup(val);
    ok = 1;
  } else if (!strcmp(name, "partitioner")) {
    if (strcmp(val, "random") == 0) {
      rd_kafka_topic_conf_set_partitioner_cb(conf.topic_conf, rd_kafka_msg_partitioner_random);
      ok = 1;
    } else if (strcmp(val, "consistent") == 0) {
      rd_kafka_topic_conf_set_partitioner_cb(conf.topic_conf, rd_kafka_msg_partitioner_consistent);
      ok = 1;
    } else {
      snprintf(errstr, errstr_size,"Unknown partitioner selected %s=%s\n Avaliable partitioners: random, consistent", name, val);
    }
  }

  if (ok) {
   // printf("Added config option %s=%s\n", name, val);
    return 0;
  }

  return -1;
}

/* Left and right trim string '*sp' of white spaces (incl newlines). */
static int trim(char **sp, char *end) {
  char *s = *sp;

  while (s < end && isspace(*s)) s++;

  end--;

  while (end > s && isspace(*end)) {
    *end = '\0';
    end--;
  }

  *sp = s;

  return (int)(end - *sp);
}

/**
 * Read and parse the supplied configuration file.
 * Returns 0 on success or -1 on failure.
 */

int read_config_file(const char *path) {
  FILE *fp;
  char buf[512];
  char errstr[512];
  int line = 0;

  if (!(fp = fopen(path, "r"))) {
    fprintf(stderr, "Failed to open configuration file %s: %s\n", path,
            strerror(errno));
    return -1;
  }

  while (fgets(buf, sizeof(buf), fp)) {
    char *s = buf;
    char *t;

    line++;

    while (isspace(*s)) s++;

    if (!*s || *s == '#') continue;

    /* "name=value"
     * find ^      */
    if (!(t = strchr(s, '='))) {
      fprintf(stderr,
              "%s:%i: warning: "
              "missing '=': line ignored\n",
              path, line);
      continue;
    }

    /* trim "name"=.. */
    if (!trim(&s, t)) {
      fprintf(stderr, "%s:%i: warning: empty left-hand-side\n", path, line);
      continue;
    }

    /* terminate "name"=.. */
    *t = '\0';
    t++;

    /* trim ..="value" */
    trim(&t, t + strlen(t));

    /* set the configuration vlaue. */
    if (conf_set(s, *t ? t : NULL, errstr, sizeof(errstr)) == -1) {
      fprintf(stderr, "%s:%i: error: %s\n", path, line, errstr);
      fclose(fp);
      return -1;
    }
  }

  fclose(fp);
  return 0;
}
