#pragma once
#include <cstdint>
#include <math.h>

// TODO: Make this an actual assetion
#define ASSERT_BREAK() *(int *)0 = 0

#if HYV_DEBUG
#define ASSERT(Expression) \
    if (!(Expression))     \
    ASSERT_BREAK()
#else
#define ASSERT(Expression)
#endif

#if HYV_DEBUG
#define VK_FUNC_ASSERT(FuncResult) \
    if (FuncResult != VK_SUCCESS)  \
    ASSERT_BREAK()
#else
#define VK_FUNC_ASSERT(FuncResult)
#endif

#if HYV_DEBUG
#define HYV_DEBUG_PRINT(Expression) std::cerr << Expression
#else
#define HYV_DEBUG_PRINT(Expression)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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

inline i16 RoundF32toI16(f32 in)
{
    return (i16)(ceilf(in - 0.5f));
}

inline i8 RoundF32toI8(f32 in)
{
    return (i8)(ceilf(in - 0.5f));
}
