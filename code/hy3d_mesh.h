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

#define MESH_INDICES_START_ADDR(m) (index *)((u8 *)(&m) + sizeof(mesh))
#define MESH_VERTICES_START_ADDR(m) (vertex *)(&m.indices[m.nIndices])

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
    // NOTE(heyyod): [mesh info][-------indices------][--------vertices--------]
    u32 nIndices;
    u32 nVertices;
    index *indices;
    vertex *vertices;
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
    meshOut->nVertices = m->position_count - 1;
    meshOut->indices = (index *)MESH_INDICES_START_ADDR((*meshOut));
    meshOut->vertices = (vertex *)MESH_VERTICES_START_ADDR((*meshOut));
    
    // NOTE(heyyod): Read all the vertex positions
    for(u32 i = 0; i < meshOut->nVertices; i++)
    {
        meshOut->vertices[i].pos = ((vec3 *)m->positions)[i+1];
        meshOut->vertices[i].texCoord = {-1.0f, -1.0f};
    }
    
    // NOTE(heyyod): For every vertex in every face set the tex coord if it has
    // not been set. If it has, append the vertices with a vertex of the same pos,
    // but a different tex coord, update the number of vertices, and update the new
    // index of the vertex on that face.
    u32 vertexIndex = 0;
    for(u32 i = 0; i < meshOut->nIndices; i++)
    {
        vertexIndex = m->indices[i].p - 1;
        vec2 readTexCoord = ((vec2 *)m->texcoords)[m->indices[i].t];
        
        if(meshOut->vertices[vertexIndex].texCoord.U < 0.0f)
        {
            meshOut->vertices[vertexIndex].texCoord = readTexCoord;
        }
        else if(meshOut->vertices[vertexIndex].texCoord.U != readTexCoord.U ||
                meshOut->vertices[vertexIndex].texCoord.V != readTexCoord.V)
        {
            meshOut->vertices[meshOut->nVertices].pos = meshOut->vertices[vertexIndex].pos;
            meshOut->vertices[meshOut->nVertices].texCoord = readTexCoord;
            meshOut->nVertices++;
            
            // NOTE(heyyod): This is always 1 greater. We correct it below
            m->indices[i].p = meshOut->nVertices;
        }
        meshOut->indices[i] = m->indices[i].p - 1;
    }
    
    fast_obj_destroy(m);
    return true;
}

#endif //HY3D_MESH_H
