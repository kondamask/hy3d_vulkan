/* date = October 28th 2021 5:43 pm */

#ifndef HY3D_SCENE_H
#define HY3D_SCENE_H

#include "mesh.h"
#include "image.h"

#define MODEL_PATH(file) ".\\assets\\models\\"##file

#define TEXTURE_PATH(file) ".\\assets\\textures\\"##file

#define SCENE_MAX_LOADED_MESHES 50

enum RESOURCE_TYPE
{
	RESOURCE_TYPE_EMPTY,

	RESOURCE_TYPE_MESH,
	RESOURCE_TYPE_TEXTURE,

	RESOURCE_TYPE_INVALID
};

struct object_transform
{
	// TODO(heyyod): This can hold rotation, scale etc so that we calculate everything in the shader
	mat4 model;
};

struct scene_ubo
{
	vec4 fogColor;        // w for exponent
	vec4 fogDistances;    // x for min, y for max, zw unused
	vec4 ambientColor;
	vec4 sunlightDir;     // w for sun power
	vec4 sunlightColor;
};

struct scene_resources
{
	void *data[MAX_STAGE_BUFFER_SLOTS];
	u64 offsets[MAX_STAGE_BUFFER_SLOTS]; // bytes from the start of the staging buffer
	RESOURCE_TYPE types[MAX_STAGE_BUFFER_SLOTS];
	u32 count;
};

struct loaded_mesh
{
	u32 count;
	u32 textureIndex;
	u32 nIndices;
	u32 nVertices;
};

struct scene_data
{
	loaded_mesh loadedMeshes[SCENE_MAX_LOADED_MESHES];
	u32 loadedMeshesCount;
	u32 loadedTexturesCount;
};

/*
static_func i32 StageResource(const char *filepath, RESOURCE_TYPE type, scene_resources &resources)
{
	u32 bytesStaged = 0;

	if (resources.count == 0)
		resources.resources[0] = resources.nextWriteAddr;

	switch (type)
	{
		case RESOURCE_MESH:
		{
			mesh *m = (mesh *)resources.nextWriteAddr;
			if (!LoadOBJ(filepath, m))
				return -1;
			resources.offsets[resources.count] = OffsetInStageBuffer(m);
			bytesStaged = MESH_PTR_TOTAL_SIZE(m);
		} break;

		case RESOURCE_TEXTURE:
		{
			image *img = (image *)resources.nextWriteAddr;
			if (!LoadImageRGBA(filepath, img))
				return -1;
			resources.offsets[resources.count] = OffsetInStageBuffer(img);
			bytesStaged = IMAGE_TOTAL_SIZE((*img));
		} break;

		default:
		{
			resources.types[resources.count] = RESOURCE_INVALID;
			DebugPrint("ERROR: Trying to stage unknown resource type.\n");
			return -1;
		}
	}
	resources.resources[resources.count] = resources.nextWriteAddr;
	resources.types[resources.count] = type;
	resources.count++;
	AdvancePointer(resources.nextWriteAddr, bytesStaged);
	return (resources.count - 1);
}
*/
// NOTE(heyyod): 
/*
I can probably have a hash map of all the loaded resources.
Also every scene can hold the names of the resources it uses.
Using the hash map I can check which resources are already loaded into the gpu.
Then if I need to load more resources that are not currently available I can
a) Allocate vram if available
b) Free unecesary resources from older scenes (LRU?)
This WILL cause unused gaps in the buffer so I need to keep this in mind!!
*/

static_func void SetNextResourceInfo(scene_resources &resources, void *data, u64 headerSize, RESOURCE_TYPE type)
{
	resources.data[resources.count] = data;
	resources.offsets[resources.count] = (u8 *)data - (u8 *)resources.data[0] + headerSize;
	resources.types[resources.count] = type;
	resources.count++;
}

#define OffsetInStageBuffer(res) (u8*)res - (u8*)resources.data[0] + sizeof(*res)
static_func i32 LoadModel(char *modelPath, char *texturePath, u32 count, void *&buffer, scene_resources &resources, scene_data &scene)
{
	// TODO: ADD GLTF LOADING
	scene.loadedMeshes[scene.loadedMeshesCount].count;
	mesh *m = (mesh *)buffer;
	if (!LoadOBJ(modelPath, m))
		return -1;
	
	SetNextResourceInfo(resources, (void *)m, sizeof(mesh), RESOURCE_TYPE_MESH);
	AdvancePointer(buffer, MESH_PTR_TOTAL_SIZE(m));

	if (texturePath)
	{
		image *img = (image *)buffer;
		if (!LoadImageRGBA(texturePath, img))
			return -1;
		SetNextResourceInfo(resources, (void *)img, sizeof(image), RESOURCE_TYPE_TEXTURE);
		AdvancePointer(buffer, IMAGE_TOTAL_SIZE((*img)));
		
		scene.loadedMeshes[scene.loadedMeshesCount].textureIndex = scene.loadedTexturesCount++;
	}
	
	scene.loadedMeshes[scene.loadedMeshesCount].count = count;
	scene.loadedMeshes[scene.loadedMeshesCount].nIndices = m->nIndices;
	scene.loadedMeshes[scene.loadedMeshesCount].nVertices = m->nVertices;
	scene.loadedMeshesCount++;
	return (scene.loadedMeshesCount - 1);
}


#define IsResourceValid(id) (id >= 0)
static_func void CreateScene(void *&buffer, scene_resources &resources, scene_data &scene, scene_ubo &sceneUBO)
{	
	i32 resourceID = -1;
	resourceID = LoadModel(MODEL_PATH("viking_room.obj"), TEXTURE_PATH("viking_room.png"), 3, buffer, resources, scene);
	Assert(resourceID >= 0);

	sceneUBO.ambientColor = { 0.5f, 0.5f, 0.65f, 0.0f };
}

#endif // HY3D_SCENE_H
