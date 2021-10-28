#ifndef HY3D_BASE_H
#define HY3D_BASE_H 1

// NOTE(heyyod): This produces a compilation error all of a sudden. wtf???? 
#include <cstdint>

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;

#define global_ static
#define local_ static
#define function_ static

#define KILOBYTES(val) (val * 1024ULL)
#define MEGABYTES(val) (KILOBYTES(val) * 1024ULL)
#define GIGABYTES(val) (MEGABYTES(val) * 1024ULL)
#define TERABYTES(val) (GIGABYTES(val) * 1024ULL)

#define Min(a, b) ((a) > (b) ? (b) : (a))
#define Max(a, b) ((a) < (b) ? (b) : (a))
#define Abs(a) ((a) > 0 ? (a) : -(a))
#define Mod(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define Square(x) ((x) * (x))

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

#define AdvancePointer(ptr, bytes) ptr = (u8*)ptr + bytes


#endif
