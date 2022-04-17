#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 1) uniform sceneData
{
	vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
};

layout(set = 0, binding = 3) uniform sampler2D textures[];


layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint textureIndex;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(textures[nonuniformEXT(textureIndex)], fragTexCoord);// * vec4(scene.ambientColor.xyz, 1.0f);
}