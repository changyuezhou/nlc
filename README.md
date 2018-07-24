# NLC (Nginx log collector)

This is a small c program that can send log lines to kafka.
NLC reads lines (\n terminated) using fgets and sends it to kafka. Before the line is sent to kafka it goes through a pluggable encoder/decoder function that can check formating, do type conversion etc. If the encoder/decoder function fails the line will be ignored by NLC (i.e. not sent to kafka).

#### Encoder/Decoder functions
- dsp_protobuf -> Expects json loglines and matches it to hardcoded protobuf spec (dsp beacon)
  - Input data is url decoded
- dynamic_protobuf -> Expects json loglines and matches to a dynamic protobuf spec (runtime loaded)
  - Inputdata is url decoded
- identity -> Forwards input string to output (i.e. no checks)
- quote -> Similar to identity but replaces single quote (') in input with double quote (") in output
- url_escape -> Similar to identity but url_escapes input data before output
- pb2tsv -> Custom encoder to convert ADN postback log to tsv format

Check "encoders.h/encoders.c" for more information and examples for how to add a new custom encoder/decoder function.

##### dynamic_protobuf encoder
dynamic_protobuf can read .proto files and load a dynamic message based on a Type name in lts properties file.
There are three configuration options for the dynamic_protobuf encoder:

- proto.type.name -> message name used in .proto file, including full package (if used) ex: com.vpon.Event 
- proto.type.folder -> .proto file folder path ex: proto_files/
- proto.type.key.name (optional) -> field name within the proto.type.name message that should be used as key.
    - If the field is a message in itself it will be serialized first.
    - Enums or Repeated fields are not supported.
    - If not set, key will always be null.


For Optional protobuf fields the json field could ither be missing or be set to json null value.
Ex:

Protobuf:
message Event {
 required string required_field = 1
 optional string optional_field = 2;
}
Valid json variants:
1. {"required_field":"important"}
2. {"required_field":"important","optional_field":null}
3. {"required_field":"important","optional_field":"some times used"}



## Operational notes
#### Log rotation
NLC expects logfiles to be rotated by date formated filenames.
This means many traditional logrotate scripts are not supported since they rely on:
1. Rename log file
2. Ask program to reopen logfile with original name

NLC does not detect the file rename and could misstake wich file it is working on.

#### Detailed example for log rotation
* f-2015-01-01.log
* f-2015-01-02.log
* f-2015-01-03.log

(Hour/Minute/Seconds format is fine too.)

NLC will pick the oldest .log (f-2015-01-01.log) file and start processing it untill EOF.
When reaching EOF, NLC will check if there are other .log files avaliable.
If so, the current file will be renamed to f-2015-01-01.log.COMPLETED and NLC will proceed to process next file (f-2015-01-02.log).

**NLC does not (currently) manage deletion of older .COMPLETE and matching .PROGESS files. This has to be done by 3rd a third party process/script or manually.**

#### Kafka send progress trackign
NLC tracks sent status to kafka on a line by line basis by storing the sent status in a seperate progress file for each .log file (file.log.PROGRESS).

EX. sent status for line nr 4 in foo.log iwll be stored on line nr 4 in foo.log.PROGRESS

Status discription:

* 0  = Not sent
* 1  = Sent
* 2  = Encoding failed
* 3  = Line buffer too small

The .PROGRESS is also used to resume after crash/restart in order to not send same lines again if possible.

** Note, there is no special status for lines that failed to be decoded/encoded. This means a logfile could still be completely sent even though there are still 0 status for some lines in the progress file.

#### Kafka and security
**Kafka protocol does not support authentication or encryption**. This means that the data from NLC to the Kafka cluster will be sent in the clear and anyone can send data to the kafka cluster if not protected at the network level.

## Code usage and main dependencies: 

* librdkafka (C client for kafka): https://github.com/edenhill/librdkafka (linked)
* jansson (C/C++ json parser): http://www.digip.org/jansson/  (linked)
* Borrows code from https://github.com/shramov/json2pb (json to protobuf encoding, MIT license)
* Borrows code from https://github.com/wikimedia/varnishkafka (config parsing, 2-clause BSD license)

## Build

#### Cmake (static build)
NLC uses cmake build script for dependency detection and cross platform build. Example release build:

    mkdir release
    cd release
    cmake /path/to/lts/soure -DCMAKE_BUILD_TYPE=Release
    make
    make install

#### Automatic build script (build_static.sh)
build_static.sh will automatically download, compile, link dependencies for NLC (zlib, librdkafka, protobuf, jansson)

    ./build_static.sh

The script will create a folder called "static". The final lts binary can be found in 'static/build/bin'

#### Dependencies:
* The GNU toolchain
* GNU make
* CMake 2.8+
* pthreads
* zlib
* rdkafka
* jansson
* protoc
* protobuf
* stdc++


### Usage and Configuraiton
By default NLC will look for lts.props in the execution folder.

    ./lts # looks for lts.props in execution folder
    ./lts -p path/to/other/file.props
    ./lts -p file.prop

There are five main configuration options in NLC

* kafka.meta.borker.list -> list of brokerip:port pairs
* watch.dir -> folder to monitor for log files
* topic -> kafka topic to send messsages to
* encoder -> encoder function to use (see Encoder/Decoder functions)
* partitioner -> kafka partitioner function to use (see Partitioner) (default: random)


librdkafka configration options can be set by the kafka. prefix in the properties file. librdkafka config options can be found here: https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md

example lts.props:

    kafka.metadata.broker.list=broker1:port1,broker2:port2,broker3:port3
    topic=topicname
    watch.dir=/path/to/log/folder/
    kafka.queue.buffering.max.messages=1000000
    kafka.message.send.max.retries=10000
    kafka.compression.codec=snappy
    kafka.topic.request.required.acks=-1
    kafka.socket.keepalive.enable=true
    partitioner=consistent
    encoder=dynamic_protobuf
    proto.type.name=Event
    proto.type.folder=/path/to/prot/files/
    proto.type.key.name=key_field
    log.level=info


**Note about request.required.acks**:
This option is used to decide what kind of producer guarantees should be used with kafka.
This option is used together with server side topic configuraiton min.isr
Please see request.required.acks description in https://kafka.apache.org/082/configuration.html
min.isr and request.acks functionality was slightly modified in kafka 0.8.2
Please see this JIRA for full details: https://issues.apache.org/jira/browse/KAFKA-1555


#### Partitioner
There are (currently) two kafka partitioners to choose from in NLC.

 * random -> random partition between 0 and max_partitions for the topic (default) (check librdkafka for implementation details)
 * consistent -> crc32 hash of the key % partition_count (check librdkafka for implementation details)


## TODO / Known issues / others

* TODO: Simplify encoder/decoder function interface (dynamically link new functions? is it useful?)
* TODO: Statistics output (network, cpu, messages sent etc)?
* TODO: Abnormal situation signaling to operations
* TODO: Systemd service file
* TODO: Add option to run as deamon?
* TODO: Message auditing?
* TODO: Read from compressed log files? (using zlib)
* TODO: Detect traditional logrotation (rename-reopen) ?
* TODO: Use inotify api on liux to resume reading after EOF instead of sleep polling?
* TODO: Delete old .COMPLETE and .PROGRESS files (should this be done by NLC?)
* TODO: Monitor multiple folders and send to different topics (or just launch many NLC?)

## Working with Nginx
The combination of Nginx and NLC creates a powerful event hub that is fast, scalable and flexible. 

#### Fast
Using ab benchmark to test a dual core 2.6ghz KVM machine with nginx+lts reaches 20 000+ reqs/sec. Network is usually the bottleneck so enabling compression in librdkafka should be considered. Especially for large messages.

#### Scalable
Vertical scalability is simple by just having mupltiple instances of the nginx+lts combination all sending to the same kafka topic

    +--------+                    +--------------+
    |Ngin|NLC+-------------------->              |
    +--------+                    |              |
    +--------+                    |              |
    |Ngin|NLC+-------------------->Kafka Cluster |
    +--------+                    |              |
    +--------+                    |              |
    |Ngin|NLC+-------------------->              |
    +--------+                    +--------------+

#### Flexible
Nginx configuration is very powerful and allows you to manipulate variables, use conditions and customize the log output (even use conditions for logging). Example usecases:

* Data validation before writing to log
* Access get parameters and conditions to detect event types
* Log formatting as csv, tsv, json etc.
* Extract cookie values by cookie name (similarly to get parameters)
* Many modules for common tasks like getting unique user ids (http://nginx.org/en/docs/http/ngx_http_userid_module.html)
* Even more advanced scripting using LUA (http://wiki.nginx.org/HttpLuaModule)
* Event serialization (avro, protobuf etc) can be achived by writing json logs in nginx and serialize it accordingly in NLC's encoder function.

#### Nginx config example


    user  nginx;

    events {
        worker_connections  1024;
    }
    
    http {
        include       mime.types;
        default_type  application/octet-stream;
    
        log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                          '$status $body_bytes_sent "$http_referer" '
                          '"$http_user_agent" "$http_x_forwarded_for"';
    
        access_log  logs/access.log  main;
        
        // Check get parameter ?cid= and validate it (-1 == invalid)
        map $arg_cid $type {
           default          -1;
           "~^d\.([^\.]+)\.(\d+)$"  2;
        }
        
        // Event log format as json. $type comes from previous map function
        log_format beacon '{"t": $type, "ts": "$msec", "cid": "$arg_cid"}';
    
        keepalive_timeout  65;
    
        // Date formated string to use for for log rotation
        map $time_iso8601 $year_and_month_day {
        "~^(?P<temp>\d{4}-\d{2}-\d{2})" $temp;
        }
    
        server {
            listen       7878;
            server_name  server.name.com;
        
            open_file_cache max=1500 inactive=30s;
            open_file_cache_valid    30s;
            open_file_cache_min_uses 2;
            open_file_cache_errors   off;
        
            location ^~ /pb {
                empty_gif;
                access_log  /var/log/nginx/log/dsp/dsp-$year_and_month_day.log  beacon;
                add_header Cache-Control "no-cache, no-store,     must-revalidate,post-check=0, pre-check=0, max-age=-1";
            }
        }
    }

#### Useful links, background, alternatives

* Core variables: http://nginx.org/en/docs/http/ngx_http_core_module.html#variables
* Nginx configuration Guide (en): http://openresty.org/download/agentzh-nginx-tutorials-en.html
* Nginx configuration Guide (cn): http://openresty.org/download/agentzh-nginx-tutorials-cn.html
* real time analytics at pinterest: http://engineering.pinterest.com/post/111380432054/real-time-analytics-at-pinterest
* pinterest log collecting tool (slinger): http://www.slideshare.net/DiscoverPinterest/singer-pinterests-logging-infrastructure
* varnish kafka used by wikipedia: https://github.com/wikimedia/varnishkafka
* wikimedia analytics including operational recommendations for kafka and other: https://wikitech.wikimedia.org/wiki/Analytics/Cluster
* lua resty kafka: https://github.com/doujiang24/lua-resty-kafka
