#version 450

layout(binding = 0) uniform testubo {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 0.0, 1.0);
    outColor = inColor;
}