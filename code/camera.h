#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include "hy3d_base.h"
#include "hy3d_math.h"

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
	
	//------------------------------------------------------------------------
	
	void Initialize(vec3 posIn, vec3 dirIn, vec3 upIn, f32 speedIn, f32 sensIn, f32 fovIn = 45.0f);
	
	void UpdateDir(f32 xOffset, f32 yOffset);
};

#endif