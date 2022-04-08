#version 450

#define GRID_VIEW_LIMIT 20.0
#define GRID_FADE_RANGE 10.0

layout(location = 0) in vec3 inFragPos;

layout( push_constant ) uniform constants
{
	vec3 playerPos;
};

layout(location = 0) out vec4 outColor;

void main()
{
	float alpha = 0.7;
	float dist = distance(inFragPos, playerPos);
	if (dist > GRID_VIEW_LIMIT)
	{
		alpha = alpha * (GRID_VIEW_LIMIT + GRID_FADE_RANGE - dist) / GRID_FADE_RANGE;
	}
	
	outColor = vec4(0.7, 0.7, 0.7, alpha);
}