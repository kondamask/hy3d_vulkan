#ifndef HY3D_BASE_H
#define HY3D_BASE_H 1

// NOTE(heyyod): This produces a compilation error all of a sudden. wtf????
#include <cstdint>

#if HY3D_DEBUG
#include <iostream>
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

#define global_var static
#define local_var static
#define static_func static

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
#define DebugPrint(val) std::cout << val
#else
#define DebugPrint(val)
#endif
	
#if HY3D_DEBUG
#define DebugPrintFunctionResult(res) std::cout << (res ? "SUCCESS: " : "FAILED :") << __FUNCTION__ << "\n"
#else
#define DebugPrintFunctionResult(res)
#endif
	
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#if HY3D_DEBUG
#define AssertArraySize(array, count) \
    if (ArrayCount(array) < count)    \
    AssertBreak()
#else
#define AssertArraySize(array, count)
#endif

#define AdvancePointer(ptr, bytes) ptr = (u8 *)ptr + bytes

#endif
