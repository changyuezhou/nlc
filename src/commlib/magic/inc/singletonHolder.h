// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_SINGLETONHOLDER_H_
#define COMMLIB_MAGIC_INC_SINGLETONHOLDER_H_

#include <string>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace magic {
    template<class T>
    class SingletonHolder {
     public:
       static T* Instance() {
         if (!instance_) {
           instance_ = new T;
         }
         return instance_;
       }

       static VOID Free() {
         if (instance_) {
           delete instance_;
           instance_ = NULL;
         }
       }

     private:
       SingletonHolder() {}
       SingletonHolder(const SingletonHolder &);
       SingletonHolder &operator=(const SingletonHolder &);
       ~SingletonHolder() {}
       static T * instance_;
    };

    template<class T>
    T * SingletonHolder<T>::instance_ = NULL;
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_SINGLETONHOLDER_H_
