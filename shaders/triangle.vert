#version 450

layout(set = 0, binding = 0) uniform cameraBuffer
{
    mat4 view;
    mat4 proj;
} cam;

struct object_data
{
	mat4 model;
};

//all object matrices
layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer
{
	object_data objects[];
} objectBuffer;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
    mat4 modelMatrix = objectBuffer.objects[0].model;
    gl_Position = cam.proj * cam.view * modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}