#pragma once
#include <cstdint>
#include <math.h>

#if HY3D_DEBUG
#include <iostream>
#endif

#define KILOBYTES(val) (val * 1024LL)
#define MEGABYTES(val) (KILOBYTES(val) * 1024LL)
#define GIGABYTES(val) (MEGABYTES(val) * 1024LL)
#define TERABYTES(val) (GIGABYTES(val) * 1024LL)

// TODO: Make this an actual assetion
#define AssertBreak() *(int *)0 = 0

#if HY3D_DEBUG
#define Assert(Expression) \
if (!(Expression))     \
AssertBreak()
#else
#define Assert(Expression)
#endif

#if HY3D_DEBUG
#define ASSERT_VK_SUCCESS(FuncResult) \
if (FuncResult != VK_SUCCESS)     \
{                                 \
DebugPrint(#FuncResult);      \
AssertBreak();                \
}
#else
#define ASSERT_VK_SUCCESS(FuncResult) \
if (FuncResult != VK_SUCCESS)     \
return false;
#endif

#if HY3D_DEBUG
#define DebugPrint(Expression) std::cerr << Expression
#else
#define DebugPrint(Expression)
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
