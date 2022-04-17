#version 450

layout(set = 0, binding = 0) uniform cameraBuffer
{
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outFragPos;

void main()
{
    gl_Position = proj * view * vec4(inPosition, 1.0);
	outFragPos = inPosition;
}