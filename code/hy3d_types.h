#pragma once
#include <cstdint>
#include <math.h>

// TODO: Make this an actual assetion
#if 1
#define ASSERT(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }
#else
#define ASSERT(Expression)
#endif

typedef int8_t  i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float  f32;
typedef double  f64;

inline i16 RoundF32toI16(f32 in)
{
    return (i16)(ceilf(in - 0.5f));
}

inline i8 RoundF32toI8(f32 in)
{
    return (i8)(ceilf(in - 0.5f));
}
