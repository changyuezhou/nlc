// Copyright (c) 2013 zhou chang yue. All rights reserved.
#include <string.h>
#include "commlib/etc/inc/conf.h"
#include "config.h"
#include <string>
#include <stdio.h>
#include "log.h"

namespace dsp {
  namespace lts {
    using lib::etc::conf;

    INT32 Config::Loading(const string & file) {
      conf etc;
      if (0 != etc.Create(file)) {
				LTS_PUB_LOG_ERROR("lts create config file failed:" << file);
        return -1;
      }

      if (0 != Loading(etc)) {
        LTS_PUB_LOG_ERROR("config file loading failed");
        return -1;
      }

      file_ = file;

      return 0;
    }

    INT32 Config::Loading(const conf & config) {

      INT32 result = LoadingWatchDir(config);
      if (0 != result) {
        LTS_PUB_LOG_ERROR("config loading watchdir failed");
        return result;
      }

      result = LoadingOtherInfo(config);
      if (0 != result) {
        LTS_PUB_LOG_ERROR("config loading other failed");
        return result;
      }

      result = LoadingNanoSleep(config);
      if (0 != result) {
				LTS_PUB_LOG_ERROR("config loading Nanosleep failed");
				return result;
      }

      result = LoadingPB(config);
      if (0 != result) {
			  LTS_PUB_LOG_ERROR("config loading PB failed");
        return result;
      }

      result = LoadingKafkaConfig(config);
      if (0 != result) {
			 	LTS_PUB_LOG_ERROR("config loading KafkaConfig failed");
        return result;
      }

      return 0;
    }

    INT32 Config::LoadingNanoSleep(const conf & config)
    {
			LTS_PUB_LOG_DEBUG("enter nanosleep");
			CHAR key[1024] = {0};
			::snprintf(key, sizeof(key), "lts.nanosleep.incr_ms");
			nano_sleep_.incr_ms = config.GetUINT64(key);

			::snprintf(key, sizeof(key), "lts.nanosleep.max_ms");
			nano_sleep_.max_ms = config.GetUINT64(key);
			return 0;
    }
    INT32 Config::LoadingWatchDir(const conf & config)
    {
			CHAR key[1024] = {0};
      ::snprintf(key, sizeof(key), "lts.watchdir.watch_dir");
      watch_dir_.watch_dir = config.GetString(key);

			if (watch_dir_.watch_dir == "")
			{
				LTS_PUB_LOG_ERROR("watch_dir_.watch_dir = NULL, or dont have this choice!");
				return -1;
			}
			return 0;
    }

    INT32 Config::LoadingOtherInfo(const conf & config)
    {
			CHAR key[1024] = {0};
      ::snprintf(key, sizeof(key), "lts.other.encoder");
      other_.encoder = config.GetString(key);

			if (other_.encoder == "")
			{
				LTS_PUB_LOG_ERROR("other_.encoder =NULL, or dont have this choice!");
				return -1;
			}	
      ::snprintf(key, sizeof(key), "lts.other.max_line_length");
      other_.max_line_len = config.GetINT32(key);
	
			return 0;
    }

    INT32 Config::LoadingPB(const conf & config) {
			LTS_PUB_LOG_DEBUG("enter pb!");
			CHAR key[1024] = {0};
			::snprintf(key, sizeof(key), "lts.proto.type_name");

			
			proto_buf_.type_name = config.GetString(key);

			if (proto_buf_.type_name == "")
			{
				LTS_PUB_LOG_ERROR("pb type_name is empty!");
				return -1;
			}


			::snprintf(key, sizeof(key), "lts.proto.type_folder");
			proto_buf_.type_folder = config.GetString(key);

			if (proto_buf_.type_folder == "")
			{
				LTS_PUB_LOG_ERROR("pb type_folder is empty!");
				return -1;
			}

			//key not necessary
			::snprintf(key, sizeof(key), "lts.proto.key_name");
			proto_buf_.key_name = config.GetString(key);
		

			return 0;
    }
    INT32 Config::LoadingKafkaConfig(const conf &config)
    {
			LTS_PUB_LOG_DEBUG("enter kafkaconfig");
			CHAR key[1024] = {0};

			::snprintf(key, sizeof(key), "lts.kafka.size");

			int a = config.GetINT32(key);
			if (a == 0)
			{
				LTS_PUB_LOG_ERROR("kafka have no paramer!");
				return -1;
			}

			int i = 0;
			KafkaConfig test;
			for (i=0;i<a;i++)
			{
				::snprintf(key, sizeof(key), "lts.kafka%d.name", i);
				test.name = config.GetString(key);
				//std::cout << test.name << std::endl;
				::snprintf(key, sizeof(key), "lts.kafka%d.value", i);
        test.value = config.GetString(key);
        //std::cout << test.value << std::endl;
				kafka_config_.push_back(test);
			}
			return 0;
    }


  }  // namespace dcc
}  // namespace dsp

