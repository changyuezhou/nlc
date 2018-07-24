// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DLL_H_
#define COMMLIB_MAGIC_INC_DLL_H_

#include <dlfcn.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/err.h"
#include "commlib/magic/inc/log.h"

namespace lib {
  namespace magic {
    using std::string;

    class DLL {
     public:
       DLL():dll_(NULL) {}
       ~DLL() { DestroyDll(); }

     public:
       INT32 CreateDll(const string & file) {
         dll_ = ::dlopen(file.c_str(), RTLD_LAZY);
         if (NULL == dll_) {
           LIB_MAGIC_LOG_ERROR("DLL dlopen file " << file << " failed errmsg:" << ::dlerror());
           return Err::kERR_MAGIC_DLL_LOAD_FILE;
         }
         return 0;
       }

       VOID * GetDllHandle() { return dll_; }

       VOID * GetFunctionHandle(const string & function) {
         return ::dlsym(dll_, function.c_str());
       }

       VOID DestroyDll() {
         if (NULL != dll_) {
           if (0 != ::dlclose(dll_)) {
             LIB_MAGIC_LOG_ERROR("DLL dlclose failed errmsg: " << ::dlerror());
           }
           dll_ = NULL;
         }
       }

     private:
       DLL(const DLL &);
       DLL &operator=(const DLL &);

     private:
       VOID * dll_;
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_DLL_H_
