// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_MAGIC_INC_DIR_H_
#define COMMLIB_MAGIC_INC_DIR_H_

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <linux/limits.h>
#include <string>
#include "commlib/public/inc/type.h"
#include "commlib/magic/inc/str.h"

namespace lib {
  namespace magic {
    using std::string;

    class Dir {
     public:
       static inline const string GetCurDir() {
         CHAR cur_dir[1024] = {0};
         ::getcwd(cur_dir, sizeof(cur_dir)-1);
         return cur_dir;
       }

       static inline const string GetBinDir() {
         CHAR bin_dir[1024] = {0};
         ::readlink("/proc/self/exe", bin_dir, sizeof(bin_dir));
         ::dirname(bin_dir);

         return bin_dir;
       }

       static inline BOOL FileExists(const string & file) {
         if (0 == ::access(file.c_str(), F_OK)) {
           return TRUE;
         }

         return FALSE;
       }

       static inline BOOL DirExists(const string & dir) {
         struct stat st;
         if (0 == ::stat(dir.c_str(), &st) && S_ISDIR(st.st_mode)) {
           return TRUE;
         }

         return FALSE;
       }

       static inline INT32 CreateDir(const string & dir, mode_t mode) {
         if (DirExists(dir)) {
           return 0;
         }

         if (0 != ::mkdir(dir.c_str(), mode)) {
           return -1;
         }

         return 0;
       }

       static inline const string GetFileName(const string & path) {
         size_t pos = path.find_last_of("/");
         if (string::npos == pos) {
           return path;
         }

         return path.substr(pos + 1, path.length());
       }

       static inline const string GetPath(const string & path) {
         size_t pos = path.find_last_of("/");
         if (string::npos == pos) {
           return "";
         }

         return path.substr(0, pos);
       }

       static inline const string GetFileExt(const string & file) {
         StringSplit split(file, ".");
         INT32 token_size = split.TokenSize();
         if (2 > token_size) {
           return "";
         }

         const string & ext = split.GetToken(token_size-1);

         if (0 == ::strncasecmp(ext.c_str(), "swp", 3)) {
           return split.GetToken(token_size-2);
         }

         return split.GetToken(token_size-1);
       }
    };
  }  // namespace magic
}  // namespace lib

#endif  // COMMLIB_MAGIC_INC_DIR_H_
