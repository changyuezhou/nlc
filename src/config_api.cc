#include "config.h"
#include "config_api.h"
#include <stdio.h>
#include <iostream>
#include "common_struct.h"
using namespace std;

using dsp::lts::Config;

using dsp::lts::NanoSleep;
using dsp::lts::WatchDir;
using dsp::lts::ProtoBuf;
using dsp::lts::KafkaConfig;
using dsp::lts::Other;
using std::vector;



Config  config_golbal;


#ifdef __cplusplus
extern "C" {
#endif

int parse_config_file(const char *fname)
{
	int ret = 0;
	//printf("parse_config_file 000!\n");
	ret = config_golbal.Loading(fname);
	//printf("parse_config_file 111!  ret=%d\n", ret);
	
	return ret;
}
void *get_data_info()
{
	return NULL;
}

//
int get_nanosleep(unsigned long *incr_ms, unsigned long *max_ms)
{
	//
	NanoSleep ns = config_golbal.GetNanoSleep();

	if (ns.incr_ms != 0)
		*incr_ms = ns.incr_ms;
	if (ns.max_ms != 0)
		*max_ms = ns.max_ms;

	return 0;
}

char *get_watchdir()
{
	WatchDir wd = config_golbal.GetWatchDir();

	return strdup(wd.watch_dir.c_str());
}

int get_protobuf_info(proto_info_c *pbinfo)
{
	ProtoBuf pb = config_golbal.GetProtoBuf();

	pbinfo->key_name = strdup(pb.key_name.c_str());
	pbinfo->type_folder = strdup(pb.type_folder.c_str());
	if (pb.type_name == "")
		pbinfo->type_name = NULL;
	else
		pbinfo->type_name = strdup(pb.type_name.c_str());

	return 0;
}

int get_other_info(other_c *otc)
{
	Other oc = config_golbal.GetOtherInfo();
	otc->encoder = strdup(oc.encoder.c_str());
	if (oc.max_line_len)
		otc->max_line_len = oc.max_line_len;
	return 0;
}

int get_kafka_config_number()
{
	return config_golbal.GetKafkaNumber();
}
//get kafka  index info
int get_kafka_index_config(int index, config_info_c *data)
{

	vector<KafkaConfig> kc = config_golbal.GetKafkaConfig();

	data->name = strdup(kc[index].name.c_str());
	data->value = strdup(kc[index].value.c_str());
	
	return 0;
}

#ifdef __cplusplus
}
#endif



