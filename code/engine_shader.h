#ifndef INCLUDE_ENGINE_SHADER_H
#define INCLUDE_ENGINE_SHADER_H

enum SHADER_RESOURCE_TYPE
{
	SHADER_RESOURCE_TYPE_UNIFORM,
	SHADER_RESOURCE_TYPE_STORAGE,
	SHADER_RESOURCE_TYPE_TEXTURE
};

enum SHADER_STAGE_FLAG
{
	SHADER_STAGE_FLAG_VERTEX = 1,
	SHADER_STAGE_FLAG_FRAGMENT = 2,
	SHADER_STAGE_FLAG_COMPUTE = 4,
};

struct shader_resource_bind
{
	SHADER_STAGE_FLAG stages;
	SHADER_RESOURCE_TYPE type;
	u32 bind;
	u64 size;
	bool perFrame; // This will create multple copies of the buffer to use on each frame on flight
	
	void **data; // This is the pointer to the buffer that we'll use.
};

#endif // INCLUDE_ENGINE_SHADER_H