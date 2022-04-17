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
	uint transformId;
};

layout(location = 0) in vec3 inPosition;

void main()
{
    mat4 modelMatrix = transform[transformId].model;
    gl_Position = proj * view * modelMatrix * vec4(inPosition, 1.0);
}