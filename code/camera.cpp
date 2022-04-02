#include "camera.h"

void camera::Initialize(vec3 posIn, vec3 dirIn, vec3 upIn, f32 speedIn, f32 sensIn, f32 fovIn)
{
	pos = posIn;
	dir = dirIn;
	up = upIn;
		
	pitch = 0.0f;
	yaw = 90.0f;
		
	speed = speedIn;
	sens = sensIn;
	fov = fovIn;
}

void camera::Update(vec3 dPos, vec2 dLook)
{
	// Position	
	pos += speed * dPos;
	
	// Look Dir
	yaw += dLook.X * sens;
	pitch += dLook.Y * sens;
	pitch = Clamp(-89.0f, pitch, 89.0f);
		
	f32 pitchRad = ToRadians(pitch);
	f32 yawRad = ToRadians(yaw);
		
	dir.X = CosF(yawRad) * CosF(pitchRad);
	dir.Y = SinF(pitchRad);
	dir.Z = SinF(yawRad) * CosF(pitchRad);
	//dir = Normalize(dir);
}