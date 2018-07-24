// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_ETC_INC_CONF_H_
#define COMMLIB_ETC_INC_CONF_H_

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include "commlib/public/inc/type.h"
#include "commlib/etc/inc/etc.h"
#include "commlib/etc/inc/err.h"

namespace lib {
  namespace etc {
    class conf: public ini {
     public:
       typedef map<string, string> Array;

     public:
       conf() {}
       virtual ~conf() {}

     protected:
       virtual INT32 LoadKeyValue(istream * fin);

     private:
       INT32 LoadItem(const string & section, istream * fin);
       INT32 LoadArray(const string & section, istream * fin);
       INT32 LoadArrayItem(INT32 index, const string & section, istream * fin);

     private:
       const conf & operator=(const conf &);
       conf(const conf &);
    };
  }  // namespace etc
}  // namespace lib

#endif  // COMMLIB_ETC_INC_CONF_H_
