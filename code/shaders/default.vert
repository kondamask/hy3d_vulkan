#version 450

struct object_transform
{
	mat4 model;
};

layout(set = 0, binding = 0) uniform cameraBuffer
{
    mat4 view;
    mat4 proj;
};

layout(std140, set = 0, binding = 2) readonly buffer object_transforms
{
	object_transform transform[];
};

layout( push_constant ) uniform constants
{
	uint transformIndex;
	uint textureIndex;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out uint outTextureIndex;

void main()
{
    mat4 modelMatrix = transform[transformIndex].model;
    gl_Position = proj * view * modelMatrix * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
	outTextureIndex = textureIndex;
}