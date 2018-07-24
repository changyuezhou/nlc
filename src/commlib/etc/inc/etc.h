// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_ETC_INC_ETC_H_
#define COMMLIB_ETC_INC_ETC_H_

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include "commlib/public/inc/type.h"

namespace lib {
  namespace etc {
    using std::string;
    using std::ifstream;
    using std::istream;
    using std::ofstream;
    using std::map;

    class etc {
     public:
       typedef map<string, string> KeyValue;

     public:
       etc() {}
       virtual ~etc() {
         SIZE_T size = key_value_.size();
         if (0 < size) {
           key_value_.clear();
         }
       }

     public:
       virtual INT32 Create(const string & file) = 0;

     public:
       const string GetString(const string & key) const;
       INT32 GetINT32(const string & key) const;
       INT32 GetUINT32(const string & key) const;
       INT32 GetINT64(const string & key) const;
       INT32 GetUINT64(const string & key) const;
       INT32 GetBOOL(const string & key) const;

     public:
       VOID Dump();

     protected:
       KeyValue key_value_;

     private:
       const etc & operator=(const etc &);
       etc(const etc &);
    };

    class ini: public etc {
     public:
       ini() {}
       virtual ~ini() {}

     public:
       virtual INT32 Create(const string & file);

     protected:
       INT32 ParseFile(const string & file);
       virtual INT32 LoadKeyValue(istream * fin);

     private:
       const ini & operator=(const ini &);
       ini(const ini &);
    };

    class xml: public etc {
     public:
       xml() {}
       virtual ~xml() {}
     public:
       virtual INT32 Create(const string & file) { return 0; }

     private:
       INT32 ParseFile(const string & file) { return 0; }

     private:
       const xml & operator=(const xml &);
       xml(const xml &);
    };

    class json: public etc {
     public:
       json() {}
       virtual ~json() {}

     public:
       virtual INT32 Create(const string & file) { return 0; }

     private:
       INT32 ParseFile(const string & file) { return 0; }

     private:
       const json & operator=(const json &);
       json(const json &);
    };
  }  // namespace etc
}  // namespace lib

#endif  // COMMLIB_ETC_INC_ETC_H_
