// Copyright (c) 2013 zhou chang yue. All rights reserved.

#include "commlib/net/inc/chttp.h"

namespace lib {
  namespace net {
    size_t CHttp::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
      call_back_param * param = reinterpret_cast<call_back_param *>(userp);
      if (NULL != param && NULL != param->data_) {
        *param->size_ = size*nmemb;
        ::memcpy(param->data_, contents, *param->size_);

        if (0 < param->notify_fd_) {
          ::write(param->notify_fd_, "I", 1);
        }
      }

      return size*nmemb;
    }

    INT32 CHttp::Request(PROTOCOL protocol, METHOD method, const HEADERS & headers, const string & host, const string & path,
                         const CHAR * data, INT32 size, CHAR * resp, INT32 * resp_size, INT32 timeout) {
      struct curl_slist * chunks = NULL;
      curl_ = curl_easy_init();

      if (curl_) {
        if (headers.size()) {
          HEADERS::const_iterator it = headers.cbegin();
          while (it != headers.cend()) {
            CHAR k_v[1024*4] = {0};
            ::snprintf(k_v, sizeof(k_v), "%s: %s", it->first.c_str(), it->second.c_str());
            chunks = ::curl_slist_append(chunks, k_v);

            it++;
          }

          ::curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunks);
        }

        if (HTTPS == protocol) {
          ::curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
          ::curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        CHAR http_host[1024] = {0};
        if (string::npos == host.find("http:")) {
          if (HTTPS == protocol) {
            ::snprintf(http_host, sizeof(http_host), "%s%s/%s", "https://", host.c_str(), path.c_str());
          } else {
            ::snprintf(http_host, sizeof(http_host), "%s%s/%s", "http://", host.c_str(), path.c_str());
          }
        } else {
          ::snprintf(http_host, sizeof(http_host), "%s/%s", host.c_str(), path.c_str());
        }
        ::curl_easy_setopt(curl_, CURLOPT_URL, http_host);
        LIB_NET_LOG_DEBUG("http host: " << http_host);

        if (0 < size) {
          ::curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data);
          ::curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, size);
        }

        if (GET == method) {
        } else if (POST == method) {
        } else if (PUT == method) {
          ::curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
        } else if (DELETE == method) {
          ::curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
        } else {
        }
      } else {
        LIB_NET_LOG_ERROR("chttp request failed: curl handler is invalid");

        return -1;
      }

      if (0 != ::pipe(pipe_)) {
        LIB_NET_LOG_ERROR("chttp create pipe failed");

        return -1;
      }

      call_back_param param;
      param.data_ = resp;
      param.size_ = resp_size;
      param.notify_fd_ = pipe_[1];

      ::curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CHttp::WriteMemoryCallback);
      ::curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &param);

      INT32 result = 0;
      CURLcode res = ::curl_easy_perform(curl_);
      if (CURLE_OK != res) {
        LIB_NET_LOG_ERROR("chttp perform request failed:" << ::curl_easy_strerror(res));
        result = -1;
      } else {
        if (0 < timeout && NULL != resp) {
          if (0 != (result = WaitForStop(pipe_[0], timeout))) {
            LIB_NET_LOG_ERROR("chttp wait for response failed");
            result = -1;
          }
        }
      }

      ::curl_easy_cleanup(curl_);

      if (chunks) {
        ::curl_slist_free_all(chunks);
      }

      if (0 < pipe_[0]) {
        ::close(pipe_[0]);
      }

      if (0 < pipe_[1]) {
        ::close(pipe_[1]);
      }

      return result;
    }

    INT32 CHttp::WaitForStop(INT32 fd, INT32 timeout) {
      fd_set rdset;
      FD_ZERO(&rdset);
      FD_SET(fd, &rdset);

      struct timeval tm;
      tm.tv_sec = timeout/1000;
      tm.tv_usec = (timeout%1000)*1000;

      if (0 < timeout) {
        if (0 < ::select(fd+1, &rdset, NULL, NULL, &tm)) {
          CHAR buf[32] = {0};
          ::read(fd, buf, sizeof(buf));
        } else {
          return -1;
        }
      }

      return 0;
    }
  }  // namespace net
}  // namespace lib
