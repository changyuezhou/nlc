// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_GEN_KEY_H_
#define COMMLIB_MAGIC_INC_GEN_KEY_H_

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/etc/inc/conf.h"

namespace lib {
  namespace magic {
    using std::string;

    class GenKey {
     public:
	 
	   
       GenKey():counter_(0) {}
       virtual ~GenKey() {}

     public:
       INT32 InitialPrefix();
       virtual const string GetID();
	  
	  

     public:
       UINT32 GetIP();
       UINT32 GetTimestamp();
       UINT32 GetPID();

     protected:
       INT32 GetCounter() { return counter_; }
       VOID ClearCounter() { counter_ = 0; }

     private:
       INT32 counter_;
       CHAR prefix_[128];
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_GEN_KEY_H_
