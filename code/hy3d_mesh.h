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

// NOTE(heyyod): SWITCH BETWEEN INDEX TYPES:
// INDEX_TYPE_U16
// INDEX_TYPE_U32
#define INDEX_TYPE_U16 1

#if INDEX_TYPE_U16
#define index_ u16
#endif

#if INDEX_TYPE_U32
#define index_ u32
#endif
typedef index_ index;

#define MESH_VERTICES_SIZE(m) \
sizeof(vertex) * m.nVertices

#define MESH_INDICES_SIZE(m) \
sizeof(index) * m.nIndices

struct mesh
{
    u32 nVertices;
    u32 nIndices;
    vertex *vertices;
    index *indices;
};

#endif //HY3D_MESH_H
