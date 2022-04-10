#version 450

layout(set = 0, binding = 0) uniform cameraBuffer
{
    mat4 view;
    mat4 proj;
} cam;

struct object_transform
{
	mat4 model;
};

layout(std140, set = 0, binding = 3) readonly buffer object_transforms
{
	object_transform transform[];
};

layout( push_constant ) uniform constants
{
	uint transformId;
} pushConstants;

layout(location = 0) in vec3 inPosition;

void main()
{
    mat4 modelMatrix = transform[pushConstants.transformId].model;
    gl_Position = cam.proj * cam.view * modelMatrix * vec4(inPosition, 1.0);
}