#version 450

layout(set = 0, binding = 2) uniform sampler2D textureSampler;

layout(set = 0, binding = 3) uniform  sceneData
{   
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} scene;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    //outColor = vec4(fragColor, 1.0f);
    outColor = texture(textureSampler, fragTexCoord * 1.0f) * vec4(scene.ambientColor.xyz, 1.0f);
}