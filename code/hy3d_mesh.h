/* date = October 14th 2021 9:57 pm */

#ifndef HY3D_MESH_H
#define HY3D_MESH_H

#include "hy3d_handmade_math.h"
#include "hy3d_base.h"

struct vertex
{
    vec3 pos;
    //vec3 normal;
    vec3 color;
};

typedef vertex vertex2; // TODO(heyyod): REMOVE THIS!

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

#define MESH_VERTICES_SIZE(m) sizeof(vertex) * m.nVertices
#define MESH_PTR_VERTICES_SIZE(m) sizeof(vertex) * m->nVertices

#define MESH_INDICES_SIZE(m) sizeof(index) * m.nIndices
#define MESH_PTR_INDICES_SIZE(m) sizeof(index) * m->nIndices

#define MESH_TOTAL_SIZE(m) MESH_VERTICES_SIZE(m) + MESH_INDICES_SIZE(m) + sizeof(m->nVertices) + sizeof(m->nIndices)
#define MESH_PTR_TOTAL_SIZE(m) sizeof(mesh) + MESH_PTR_VERTICES_SIZE(m) + MESH_PTR_INDICES_SIZE(m)

#define MESH_PTR_VERTICES_START_ADDR(m) (vertex *)((u8 *)m + sizeof(mesh))
#define MESH_PTR_INDICES_START_ADDR(m) (index *)(&m->vertices[m->nVertices])

#define MESH_PTR_DATA_START_OFFSET sizeof(mesh)

struct mesh
{
    u32 nVertices;
    u32 nIndices;
    vertex *vertices;
    index *indices;
};

#endif //HY3D_MESH_H
