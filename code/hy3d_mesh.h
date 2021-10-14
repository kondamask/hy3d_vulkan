/* date = October 14th 2021 9:57 pm */

#ifndef HY3D_MESH_H
#define HY3D_MESH_H

#include "hy3d_math.h"

struct vertex
{
    vec2 pos;
    //vec3 normal;
    vec3 color;
};

struct mesh
{
    u32 nVertices;
    vertex *vertices;
    //u32 *indices;
};

#endif //HY3D_MESH_H
