#include "core.h"
#include "engine_platform.h"
#include "asset.h"
#include "math.h"
#include "mesh.h"

static_func void EngineViewCreateGrid(void *buffer, u64 &offset, asset *asset)
{
	asset->type = ASSET_TYPE_MESH;

	asset->header = buffer;
	mesh_header *m = (mesh_header *)asset->header;
	m->vertexSize = sizeof(vec3);
	m->vertices = (u8 *)m + sizeof(*m);

	// Create Grid Vertices
	vec3 *v = (vec3 *)m->vertices;
	u32 iVert = 0;
	i32 gridSize = 500;
	i32 halfGridSize = gridSize / 2;
	v[iVert++] = { 0.0f, (f32)halfGridSize, 0.0f };
	v[iVert++] = { 0.0f, -(f32)halfGridSize, 0.0f };
	{
		i32 z = halfGridSize;
		for (i32 x = -halfGridSize; x < halfGridSize; x++)
		{
			v[iVert++] = { (f32)x, 0.0f, (f32)z };
			v[iVert++] = { (f32)x, 0.0f, -(f32)z };
		}
	}
	{
		i32 x = halfGridSize;
		for (i32 z = -halfGridSize; z < halfGridSize; z++)
		{
			v[iVert++] = { (f32)x, 0.0f, (f32)z };
			v[iVert++] = { -(f32)x, 0.0f, (f32)z };
		}
	}

	m->nVertices = iVert - 1;

	u64 size = MESH_PTR_TOTAL_SIZE(m);
	offset += size;

	AdvancePointer(buffer, size);
}

static_func bool EngineViewInitialize(engine_platform *engine)
{
	// TODO: Remove the vulkan related stuff
	void *buffer = VulkanRequestBuffer(RENDERER_BUFFER_TYPE_STAGING, MEGABYTES(64), 0);
	u64 offset = 0;

	asset assets[10] = {};

	EngineViewCreateGrid(buffer, offset, &assets[0]);

	// TODO: UI stuff, AABBs, more?

	VulkanUpload(assets, 1);
	VulkanClearBuffer(vulkanContext->stagingBuffer);
}