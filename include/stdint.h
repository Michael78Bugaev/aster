#pragma once
#include <stddef.h>
/* Most commonly used types */
#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef bool
#define bool int
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)(value) < 0)

/* stdint types */
typedef unsigned char       uint8_t;
typedef   signed char        int8_t;
typedef unsigned short     uint16_t;
typedef   signed short      int16_t;
typedef unsigned int       uint32_t;
typedef   signed int        int32_t;
typedef unsigned long long uint64_t;
typedef   signed long long  int64_t;
typedef unsigned long     uintptr_t;
typedef   signed long      intptr_t;

/* those are commonly provided by sys/types.h */
typedef unsigned long         ino_t;
typedef unsigned int         mode_t;
typedef   signed int          pid_t;
typedef unsigned int          uid_t;
typedef unsigned int          gid_t;
typedef   signed long         off_t;
typedef   signed long     blksize_t;
typedef   signed long      blkcnt_t;
typedef   signed long        time_t;

//typedef signed long ssize_t;