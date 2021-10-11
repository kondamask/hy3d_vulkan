#pragma once
#include "hy3d_base.h"
#include "math.h"

inline f32 minF32(f32 a, f32 b)
{
    if (a <= b)
        return a;
    return b;
}

inline f32 maxF32(f32 a, f32 b)
{
    if (a >= b)
        return a;
    return b;
}

inline f32 Squared(f32 n)
{
    return n * n;
}

struct vec3
{
    union
    {
        struct
        {
            f32 x, y, z;
        };
        struct
        {
            f32 r, g, b;
        };
        f32 pos[3];
    };
    
    inline vec3 operator=(vec3 b)
    {
        x = b.x;
        y = b.y;
        z = b.z;
        return *this;
    }
    
    inline vec3 operator/(f32 b)
    {
        return vec3{x / b, y / b, z / b};
    }
    
    inline f32 lengthSq()
    {
        return (x * x + y * y + z * z);
    }
    
    inline f32 length()
    {
        return sqrtf(lengthSq());
    }
    
    inline vec3 normalized()
    {
        f32 l = length();
        if (l == 0.0f)
            return {};
        return (*this / l);
    }
    
    inline void normalize()
    {
        *this = this->normalized();
    }
};

typedef vec3 colorV3;

// NOTE:  addition, subtraction
inline vec3 operator+(vec3 a, vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline vec3 operator-(vec3 a, vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline vec3 operator+=(vec3 &a, vec3 b)
{
    a = a + b;
    return a;
}

inline vec3 operator-=(vec3 &a, vec3 b)
{
    a = a - b;
    return a;
}

// NOTE:  vector * number
inline vec3 operator*(f32 a, vec3 b)
{
    return {a * b.x, a * b.y, a * b.z};
}

inline vec3 operator*(vec3 b, f32 a)
{
    return a * b;
}

inline vec3 operator*=(vec3 &a, f32 b)
{
    a = b * a;
    return a;
}

// NOTE:  negative of a vector
inline vec3 operator-(vec3 a)
{
    return {-a.x, -a.y, -a.z};
}

// NOTE:  dot product
inline f32 operator*(vec3 a, vec3 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline f32 DotProduct(vec3 a, vec3 b)
{
    return (a * b);
}

inline vec3 CrossProduct(vec3 a, vec3 b)
{
    return {a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

inline vec3 HadamardProduct(vec3 a, vec3 b)
{
    return {a.x * b.x,
        a.y * b.y,
        a.z * b.z};
}

inline vec3 Saturated(vec3 a)
{
    a.x = minF32(1.0f, maxF32(0.0f, a.x));
    a.y = minF32(1.0f, maxF32(0.0f, a.y));
    a.z = minF32(1.0f, maxF32(0.0f, a.z));
    return a;
}

// NOTE:  comparison
inline bool operator==(vec3 a, vec3 b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline bool operator!=(vec3 a, vec3 b)
{
    return !(a == b);
}

inline vec3 lerp(vec3 P, vec3 A, vec3 B, f32 alpha)
{
    return P + (B - P) * alpha;
}

// ----------------------------------------------------------------------------------------
// NOTE:  VEC2 ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

struct vec2
{
    union
    {
        struct
        {
            f32 x, y;
        };
        f32 pos[2];
    };
    
    inline vec2 operator=(vec2 b)
    {
        x = b.x;
        y = b.y;
        return *this;
    }
    
    inline vec2 operator-(vec2 b)
    {
        return vec2{x - b.x, y - b.y};
    }
    
    inline vec2 operator/(f32 b)
    {
        return vec2{x / b, y / b};
    }
    
    inline f32 lengthSq()
    {
        return (x * x + y * y);
    }
    
    inline f32 length()
    {
        return sqrtf(lengthSq());
    }
    
    inline vec2 normal()
    {
        f32 l = length();
        if (l == 0.0f)
            return {};
        return (*this / l);
    }
    
    inline void normalize()
    {
        *this = this->normal();
    }
};

// NOTE:  addition, subtraction
inline vec2 operator+(vec2 a, vec2 b)
{
    return {a.x + b.x, a.y + b.y};
}

inline vec2 operator+=(vec2 &a, vec2 b)
{
    a = a + b;
    return a;
}

inline vec2 operator-=(vec2 &a, vec2 b)
{
    a = a - b;
    return a;
}

// NOTE:  vector * number
inline vec2 operator*(f32 a, vec2 b)
{
    return {a * b.x, a * b.y};
}

inline vec2 operator*(vec2 b, f32 a)
{
    return a * b;
}

inline vec2 operator*=(vec2 &a, f32 b)
{
    a = b * a;
    return a;
}

// NOTE:  negative of a vector
inline vec2 operator-(vec2 a)
{
    return {-a.x, -a.y};
}

// NOTE:  dot product
inline f32 operator*(vec2 a, vec2 b)
{
    return (a.x * b.x + a.y * b.y);
}

inline f32 DotProduct(vec2 a, vec2 b)
{
    return (a * b);
}

// NOTE:  comparison
inline bool operator==(vec2 a, vec2 b)
{
    return (a.x == b.x && a.y == b.y);
}

inline bool operator!=(vec2 a, vec2 b)
{
    return !(a == b);
}

inline vec2 lerp(vec2 P, vec2 A, vec2 B, f32 alpha)
{
    return P + (B - P) * alpha;
}

struct mat3
{
    float cell[3][3]; // [row][column]
};
/*
 function mat3 Identity()
{
    return {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f};
}
*/
function mat3 Scale(float factor)
{
    return {
        factor, 0.0f, 0.0f,
        0.0f, factor, 0.0f,
        0.0f, 0.0f, factor};
}

function mat3 RotateZ(float theta)
{
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    return {
        cosTheta, sinTheta, 0.0f,
        -sinTheta, cosTheta, 0.0f,
        0.0f, 0.0f, 1.0f};
}

function mat3 RotateY(float theta)
{
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    return {
        cosTheta, 0.0f, -sinTheta,
        0.0f, 1.0f, 0.0f,
        sinTheta, 0.0f, cosTheta};
}

function mat3 RotateX(float theta)
{
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    return {
        1.0f, 0.0f, 0.0f,
        0.0f, cosTheta, sinTheta,
        0.0f, -sinTheta, cosTheta};
}

mat3 operator*(mat3 a, mat3 b)
{
    mat3 result;
    for (size_t j = 0; j < 3; j++)
    {
        for (size_t k = 0; k < 3; k++)
        {
            float sum = 0.0f;
            for (size_t i = 0; i < 3; i++)
            {
                sum += a.cell[j][i] * b.cell[i][k];
            }
            result.cell[j][k] = sum;
        }
    }
    return result;
}

mat3 operator*=(mat3 &a, mat3 b)
{
    a = a * b;
    return a;
}

mat3 operator*(mat3 a, float b)
{
    for (size_t j = 0; j < 3; j++)
    {
        for (size_t i = 0; i < 3; i++)
        {
            a.cell[i][j] *= b;
        }
    }
    return a;
}

mat3 operator*(float b, mat3 a)
{
    return a * b;
}

mat3 operator*=(mat3 &a, float b)
{
    a = a * b;
    return a;
}

vec3 operator*(vec3 v, mat3 m)
{
    vec3 result = {};
    result.x = v.x * m.cell[0][0] + v.y * m.cell[1][0] + v.z * m.cell[2][0];
    result.y = v.x * m.cell[0][1] + v.y * m.cell[1][1] + v.z * m.cell[2][1];
    result.z = v.x * m.cell[0][2] + v.y * m.cell[1][2] + v.z * m.cell[2][2];
    return result;
}

vec3 operator*=(vec3 &v, mat3 m)
{
    v = v * m;
    return v;
}