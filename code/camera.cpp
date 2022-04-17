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

static_func void CameraInitialize(camera &cam, vec3 posIn, f32 pitch, f32 yaw, vec3 up, f32 speed, f32 sens, f32 fov)
{
	cam.pos = posIn;
	cam.up = up;
		
	cam.pitch = pitch;
	cam.yaw = yaw;
	CameraUpdate(cam, {}, {});
		
	cam.speed = speed;
	cam.sens = sens;
	cam.fov = fov;
}

