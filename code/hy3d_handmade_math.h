/* date = October 15th 2021 8:14 pm */

#ifndef HY3D_HANDMADE_MATH_H
#define HY3D_HANDMADE_MATH_H

#include "libs/HandmadeMath.h"
#include "hy3d_base.h"

typedef hmm_vec2 vec2;
typedef hmm_vec3 vec3;
typedef hmm_vec4 vec4;
typedef hmm_mat4 mat4;
typedef hmm_quaternion quart;

inline i16 RoundF32toI16(f32 in)
{
    return (i16)(ceilf(in - 0.5f));
}

inline i8 RoundF32toI8(f32 in)
{
    return (i8)(ceilf(in - 0.5f));
}

#endif //HY3D_HANDMADE_MATH_H
