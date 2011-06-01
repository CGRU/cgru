#pragma once

#ifdef WINNT
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef _int64 int64_t;
//typedef unsigned long long   uint64_t;
#else
#include <stdint.h>
#endif
