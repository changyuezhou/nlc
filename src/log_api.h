#ifndef LTS_PUBLIC_SRC_H
#define LTS_PUBLIC_SRC_H

#ifdef __cplusplus
extern "C" {
#endif

int parser_log_config(char *fname);

void lts_public_log_debug(char *expression);

void lts_public_log_info(char *expression);

void lts_public_log_warn(char *expression);

void lts_public_log_error(char *expression);

void lts_public_log_fatal(char *expression);



#ifdef __cplusplus
}
#endif



#endif
