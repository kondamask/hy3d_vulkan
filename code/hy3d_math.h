/* date = October 15th 2021 8:14 pm */

#ifndef HY3D_MATH_H
#define HY3D_MATH_H

#include "libs/HandmadeMath.h"
#include "hy3d_base.h"

#define VEC3_UP { 0.0f, 1.0f, 0.0f }
#define VEC3_DOWN { 0.0f, -1.0f, 0.0f }
#define VEC3_RIGHT { 1.0f, 0.0f, 0.0f }
#define VEC3_LEFT { -1.0f, 1.0f, 0.0f }
#define VEC3_FRONT { 0.0f, 0.0f, 1.0f }
#define VEC3_BACK { 0.0f, 0.0f, -1.0f }

typedef hmm_vec2 vec2;
typedef hmm_vec3 vec3;
typedef hmm_vec4 vec4;
typedef hmm_mat4 mat4;
typedef hmm_quaternion quart;

#define Sign(a) (a > 0) ? 1 : -1

inline static_func f32 Wrap(f32 min, f32 val, f32 max)
{
    f32 result = val;
    if(val < min)
    {
        result = max - (min - val);
    }
    else if (val > max)
    {
        result = min + (val - max);
    }
    return result;
}

inline i16 RoundF32toI16(f32 in)
{
    return (i16)(ceilf(in - 0.5f));
}

inline i8 RoundF32toI8(f32 in)
{
    return (i8)(ceilf(in - 0.5f));
}

#endif //HY3D_MATH_H