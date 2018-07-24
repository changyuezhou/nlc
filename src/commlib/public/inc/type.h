// Copyright (c) 2013 zhou chang yue. All rights reserved.

#ifndef COMMLIB_PUBLIC_INC_TYPE_H_
#define COMMLIB_PUBLIC_INC_TYPE_H_

#include <sys/shm.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stddef.h>

typedef unsigned char UCHAR;
typedef char CHAR;
typedef char INT8;
typedef unsigned char UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef int16_t SHORT;
typedef uint16_t USHORT;
typedef uint32_t UINT32;
typedef int64_t LONG;
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef void VOID;
typedef bool BOOL;
typedef size_t SIZE_T;
typedef time_t TIME_T;
typedef socklen_t SOCKLEN_T;
typedef int32_t SOCKET;
typedef key_t KEY_T;

const BOOL TRUE = true;
const BOOL FALSE = false;

const INT32 INVALID_SOCKET_HANDLE = -1;

#endif  // COMMLIB_PUBLIC_INC_TYPE_H_
