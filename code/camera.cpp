#include "camera.h"

static_func void CameraUpdate(camera &cam, vec3 dPos, vec2 dLook)
{
	// Position	
	cam.pos += cam.speed * dPos;
	
	// Look Dir
	cam.yaw += dLook.X * cam.sens;
	if (cam.yaw > 360.0f)
		cam.yaw -= 360.0f;
	else if (cam.yaw < 0.0f)
		cam.yaw = 360.0f - cam.yaw;
	
	cam.pitch += dLook.Y * cam.sens;
	cam.pitch = Clamp(-89.0f, cam.pitch, 89.0f);
	
	f32 pitchRad = ToRadians(cam.pitch);
	f32 yawRad = ToRadians(cam.yaw);
		
	cam.dir.X = CosF(yawRad) * CosF(pitchRad);
	cam.dir.Y = SinF(pitchRad);
	cam.dir.Z = SinF(yawRad) * CosF(pitchRad);
}

static_func void CameraInitialize(camera &cam, vec3 posIn, vec3 dirIn, vec3 upIn, f32 speedIn, f32 sensIn, f32 fovIn)
{
	cam.pos = posIn;
	cam.dir = dirIn;
	cam.up = upIn;
		
	cam.pitch = 0.0f;
	cam.yaw = 90.0f;
	CameraUpdate(cam, {}, {});
		
	cam.speed = speedIn;
	cam.sens = sensIn;
	cam.fov = fovIn;
}

