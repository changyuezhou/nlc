#ifndef CONFIG_API_H
#define CONFIG_API_H

#include "common_struct.h"
#ifdef __cplusplus
extern "C" {
#endif

void *get_data_info();
int parse_config_file(const char *fname);


int get_nanosleep(unsigned long *incr_ms, unsigned long *max_ms);
char *get_watchdir();
int get_protobuf_info(proto_info_c *pbinfo);
int get_other_info(other_c *otc);
int get_kafka_index_config(int index, config_info_c *data);




#ifdef __cplusplus
}
#endif

#endif
