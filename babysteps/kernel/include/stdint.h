#ifndef _STDINT_H
#define _STDINT_H

/* Exact-width integer types (compiler built-ins for portability) */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

/* Fast and least types (optional) */
typedef int32_t  int_fast32_t;
typedef uint32_t uint_fast32_t;

/* Pointer-sized integer types */
#if SIZEOF_POINTER == 8
typedef int64_t  intptr_t;
typedef uint64_t uintptr_t;
#elif SIZEOF_POINTER == 4
typedef int32_t  intptr_t;
typedef uint32_t uintptr_t;
#endif

/* Limits (optional minimal set) */
#define INT8_MIN   (-128)
#define INT8_MAX   127
#define UINT8_MAX  255
#define INT16_MIN  (-32768)
#define INT16_MAX  32767
#define UINT16_MAX 65535u
#define INT32_MIN  (-2147483647-1)
#define INT32_MAX  2147483647
#define UINT32_MAX 4294967295u
/* 64-bit limits omitted for brevity */

#endif /* _STDINT_H */
