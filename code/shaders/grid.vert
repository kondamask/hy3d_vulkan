#version 450

layout(set = 0, binding = 0) uniform cameraBuffer
{
    mat4 view;
    mat4 proj;
} cam;

layout(location = 0) in vec3 inPosition;

void main()
{
    gl_Position = cam.proj * cam.view * vec4(inPosition, 1.0);
}