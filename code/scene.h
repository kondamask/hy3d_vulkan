/* date = October 28th 2021 5:43 pm */

#ifndef HY3D_SCENE_H
#define HY3D_SCENE_H

#include "mesh.h"
#include "image.h"
#include "asset.h"
#include "renderer_buffer.h"

#define MODEL_PATH(file) ".\\assets\\models\\"##file

#define TEXTURE_PATH(file) ".\\assets\\textures\\"##file

#define SCENE_MAX_LOADED_MESHES 50

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

struct loaded_mesh
{
	u32 instances;
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

#endif