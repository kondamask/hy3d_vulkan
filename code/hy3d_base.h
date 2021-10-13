#ifndef HY3D_BASE_H
#define HY3D_BASE_H 1

#include <cstdint>
#include <math.h>

#define KILOBYTES(val) (val * 1024ULL)
#define MEGABYTES(val) (KILOBYTES(val) * 1024ULL)
#define GIGABYTES(val) (MEGABYTES(val) * 1024ULL)
#define TERABYTES(val) (GIGABYTES(val) * 1024ULL)

// TODO: Make this an actual assetion
#define AssertBreak() *(int *)0 = 0

#if HY3D_DEBUG
#define Assert(val) \
if (!(val))     \
AssertBreak()
#else
#define Assert(val)
#endif

#if HY3D_DEBUG
// TODO(heyyod): Use fast_io instead?
#include <iostream>
#define DebugPrint(val) std::cerr << val
#else
#define DebugPrint(val)
#endif

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#if HY3D_DEBUG
#define AssertArraySize(array, count) \
if (ArrayCount(array) < count)     \
AssertBreak()
#else
#define AssertArraySize(array, count)
#endif


typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define global static
#define local static
#define function static

inline i16 RoundF32toI16(f32 in)
{
    return (i16)(ceilf(in - 0.5f));
}

inline i8 RoundF32toI8(f32 in)
{
    return (i8)(ceilf(in - 0.5f));
}

#endif
