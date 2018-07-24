#ifndef DSP_LTS_INC_CONFIG_H_
#define DSP_LTS_INC_CONFIG_H_

#include <stdlib.h>
#include <string.h>
#include "commlib/public/inc/type.h"
#include "commlib/etc/inc/conf.h"
//#include "dsp/public/inc/log.h"
//#include "dsp/public/inc/err.h"
#include <vector>
#include <string>

namespace dsp{
	namespace lts{
		using std::string;
		using lib::etc::conf;
		using std::vector;


		typedef struct _NanoSleep
		{
			unsigned long incr_ms;
			unsigned long max_ms;
		}NanoSleep;
		
		typedef struct _WatchDir
		{
			string watch_dir;
		}WatchDir;

		
		typedef struct _ProtoBuf
		{
			string type_name;
			string type_folder;
			string key_name;
		}ProtoBuf;

		typedef struct _KafkaConfig
		{
			string name;
			string value;
		}KafkaConfig;

		typedef struct _Other
		{
			string encoder;
			INT32 max_line_len;
		}Other;	


		class Config {
		public:
			Config() {}
			~Config() {}

			public:
			INT32 Loading(const string & file);
			INT32 Loading(const conf & config);

			INT32 LoadingWatchDir(const conf & config);
			INT32 LoadingNanoSleep(const conf & config);
			INT32 LoadingPB(const conf & config);
			INT32 LoadingKafkaConfig(const conf & config);
			INT32 LoadingOtherInfo(const conf & config);
			

		public:
			const ProtoBuf & GetProtoBuf() { return proto_buf_; }
			const vector<KafkaConfig> & GetKafkaConfig(){return kafka_config_;};
			const WatchDir & GetWatchDir(){return watch_dir_;}
			const NanoSleep & GetNanoSleep(){return nano_sleep_;}
			const int GetKafkaNumber(){return kafka_config_.size();}
			const Other GetOtherInfo(){return other_;}

		private:
			string file_;
			ProtoBuf proto_buf_;
			vector<KafkaConfig> kafka_config_;
			NanoSleep nano_sleep_;
			WatchDir  watch_dir_;
			Other	other_;
		};
		
	}

}




#endif

