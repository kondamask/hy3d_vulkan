#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include "core.h"
#include "math.h"

struct camera
{
	vec3 pos;
	vec3 dir;
	f32 speed;
	f32 fov;
	vec3 up;
	f32 pitch;
	f32 yaw;
	f32 sens;
};

struct camera_ubo
{
	mat4 view;
	mat4 proj;
};

#endif