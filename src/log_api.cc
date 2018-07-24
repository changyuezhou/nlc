#include "commlib/log/inc/handleManager.h"
#include "log.h"


using lib::log::HandleManager;

#ifdef __cplusplus
extern "C" {
#endif

int parser_log_config(char *fname)
{
	int result = HandleManager::GetInstance()->Initial(fname);
	return result;
}

void lts_public_log_debug(char *expression)
{
	LTS_PUB_LOG_DEBUG(expression);
}

void lts_public_log_info(char *expression)
{
	LTS_PUB_LOG_INFO(expression);
}

void lts_public_log_warn(char *expression)
{
	LTS_PUB_LOG_WARN(expression);
}

void lts_public_log_error(char *expression)
{
	LTS_PUB_LOG_ERROR(expression);
}

void lts_public_log_fatal(char *expression)
{
	LTS_PUB_LOG_FATAL(expression);
}

#ifdef __cplusplus
}
#endif
