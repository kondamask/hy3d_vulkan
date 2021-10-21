/* date = October 14th 2021 9:57 pm */

#ifndef HY3D_MESH_H
#define HY3D_MESH_H

#include "hy3d_math.h"
#include "hy3d_base.h"

#define FAST_OBJ_IMPLEMENTATION
#include "libs/fast_obj.h"

// NOTE(heyyod): SWITCH BETWEEN INDEX TYPES:
// INDEX_TYPE_U16
// INDEX_TYPE_U32
#define INDEX_TYPE_U32 1

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

struct vertex
{
    vec3 pos;
    vec3 color;
    vec2 texCoord;
    //vec3 normal;
};

struct mesh
{
    u32 nVertices;
    u32 nIndices;
    vertex *vertices;
    index *indices;
};

function bool LoadOBJ(const char *filename, mesh *meshOut)
{
    fastObjMesh* m = fast_obj_read(filename);
    if(!m)
    {
        DebugPrint("ERROR: Could not load obj: ");
        DebugPrint(filename);
        return false;
    }
    meshOut->nIndices = 3 * m->face_count;
    meshOut->nVertices = meshOut->nIndices;
    meshOut->vertices = (vertex *)MESH_PTR_VERTICES_START_ADDR(meshOut);
    meshOut->indices = (index *)MESH_PTR_INDICES_START_ADDR(meshOut);
    for(u32 i = 0; i < meshOut->nIndices; i++)
    {
        meshOut->vertices[i].pos = ((vec3 *)m->positions)[m->indices[i].p];
        meshOut->vertices[i].color = ((vec3 *)m->normals)[m->indices[i].n];
        meshOut->vertices[i].texCoord = ((vec2 *)m->texcoords)[m->indices[i].t];
        meshOut->indices[i] = i;
    }
    /* 
meshOut->nVertices = m->position_count - 1;
    meshOut->nIndices = 3 * m->face_count;
    meshOut->vertices = (vertex *)MESH_PTR_VERTICES_START_ADDR(meshOut);
    meshOut->indices = (index *)MESH_PTR_INDICES_START_ADDR(meshOut);
        for(u32 i = 0; i < meshOut->nVertices; i++)
        {
            meshOut->vertices[i].pos = ((vec3 *)m->positions)[i+1];
            meshOut->vertices[i].color = ((vec3 *)m->normals)[i+1];
            meshOut->vertices[i].texCoord = ((vec2 *)m->texcoords)[i+1];
            meshOut->vertices[i].texCoord.Y = 1.0f - meshOut->vertices[i].texCoord.Y;
        }
        for(u32 i = 0; i < meshOut->nIndices; i++)
        {
            meshOut->indices[i] = m->indices[i].p - 1;
        }
     */
    fast_obj_destroy(m);
    return true;
}

#endif //HY3D_MESH_H
