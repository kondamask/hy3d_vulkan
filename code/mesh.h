#ifndef HY3D_MESH_H
#define HY3D_MESH_H

#include "math.h"
#include "core.h"
#include "asset.h"

#define FAST_OBJ_IMPLEMENTATION
#include "extern/fast_obj.h"

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

#define MESH_VERTICES_SIZE(m) m.vertexSize * m.nVertices
#define MESH_PTR_VERTICES_SIZE(m) m->vertexSize * m->nVertices

#define MESH_INDICES_SIZE(m) sizeof(index) * m.nIndices
#define MESH_PTR_INDICES_SIZE(m) sizeof(index) * m->nIndices

#define MESH_TOTAL_SIZE(m) MESH_VERTICES_SIZE(m) + MESH_INDICES_SIZE(m) + sizeof(mesh_header)
#define MESH_PTR_TOTAL_SIZE(m) sizeof(mesh_header) + MESH_PTR_VERTICES_SIZE(m) + MESH_PTR_INDICES_SIZE(m)

#define MESH_INDICES_START_ADDR(m) (index *)((u8 *)(&m) + sizeof(mesh_header))
#define MESH_VERTICES_START_ADDR(m) (void *)(&m.indices[m.nIndices])

#define MESH_PTR_DATA_START_OFFSET sizeof(mesh_header)

struct vertex
{
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
};

struct mesh_header
{
	// NOTE(heyyod): [mesh header][-------indices------][--------vertices--------]
	u32 nIndices;
	u32 nVertices;
	u64 indicesOffset;
	u64 verticesOffset;
	u32 vertexSize;
	index *indices;
	void *vertices; // This should be void* because we do not know what kind of info each mesh's vertex might contain
};

static_func bool LoadOBJ(const char *filename, asset *asset)
{
	fastObjMesh* m = fast_obj_read(filename);
	if (!m)
	{
		DebugPrint("ERROR: Could not load obj: ");
		DebugPrint(filename);
		return false;
	}

	asset->type = ASSET_TYPE_MESH;
	mesh_header *mesh = (mesh_header *)asset->header;
	mesh->nIndices = 3 * m->face_count;
	mesh->nVertices = m->position_count - 1;
	mesh->indices = (index *)MESH_INDICES_START_ADDR((*mesh));
	mesh->vertices = MESH_VERTICES_START_ADDR((*mesh));

	vertex *vertices = (vertex *)(mesh->vertices);

	// NOTE(heyyod): Read all the vertex positions
	for (u32 i = 0; i < mesh->nVertices; i++)
	{
		vertices[i].pos = ((vec3 *)m->positions)[i + 1];
		vertices[i].texCoord.U = -1.0f;
	}

	// NOTE(heyyod): For every vertex in every face set the tex coord if it has
	// not been set. If it has, append the vertices with a vertex of the same pos,
	// but a different tex coord, update the number of vertices, and update the new
	// index of the vertex on that face.
	u32 vertexIndex = 0;
	for (u32 i = 0; i < mesh->nIndices; i++)
	{
		vertexIndex = m->indices[i].p - 1;
		vec2 readTexCoord = ((vec2 *)m->texcoords)[m->indices[i].t];
		readTexCoord.V = 1.0f - readTexCoord.V;

		if (vertices[vertexIndex].texCoord.U < 0.0f)
		{
			vertices[vertexIndex].texCoord = readTexCoord;
		}
		else if (vertices[vertexIndex].texCoord.U != readTexCoord.U ||
			vertices[vertexIndex].texCoord.V != readTexCoord.V)
		{
			vertices[mesh->nVertices].pos = vertices[vertexIndex].pos;
			vertices[mesh->nVertices].texCoord = readTexCoord;
			mesh->nVertices++;

			// NOTE(heyyod): This is always 1 greater. We correct it below
			m->indices[i].p = mesh->nVertices;
		}
		mesh->indices[i] = m->indices[i].p - 1;
	}

	fast_obj_destroy(m);

	// TODO: Mesh Optimizer: https://github.com/zeux/meshoptimizer

	// Normalize:
	f32 maxPos = -F32_MAX;
	f32 minPos = F32_MAX;
	for (u32 i = 0; i < mesh->nVertices; i++)
	{
		f32 X = vertices[i].pos.X;
		f32 Y = vertices[i].pos.Y;
		f32 Z = vertices[i].pos.Z;
		
		if (X > maxPos)
			maxPos = X;
		if (Y > maxPos)
			maxPos = Y;
		if (Z > maxPos)
			maxPos = Z;
		
		if (X < minPos)
			minPos = X;
		if (Y < minPos)
			minPos = Y;
		if (Z < minPos)
			minPos = Z;
	}
	f32 diff = maxPos - minPos;
	if (diff != 1.0f)
	{			
		for (u32 i = 0; i < mesh->nVertices; i++)
		{
			vertices[i].pos.X /= diff;
			vertices[i].pos.Y /= diff;
			vertices[i].pos.Z /= diff;
		}
	}
	
	return true;
}

#endif // HY3D_MESH_H
