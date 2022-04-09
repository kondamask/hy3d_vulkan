#ifndef INCLUDE_RENDERER_PLATFORM_H
#define INCLUDE_RENDERER_PLATFORM_H

#include "core.h"
#include "math.h"
#include "graphics_settings.h"

//------------------------------------------------------------------------

#define GRAPHICS_CONTEXT_SIZE KILOBYTES(5)

struct graphics_context
{
	u8 data[GRAPHICS_CONTEXT_SIZE];
};


//------------------------------------------------------------------------
struct engine_state;

enum RENDERER_GRAPHICS_API
{
	RENDERER_GRAPHICS_API_VULKAN,
	RENDERER_GRAPHICS_API_OPENGL,
	RENDERER_GRAPHICS_API_DIRECTX,
};

struct render_packet
{
	// ENGINE -> RENDERER
	f32 dt;
	vec3 clearColor;
	vec3 playerPos;
    
	//------------------------------------------------------------------------
	
	// RENDERER -> ENGINE
	// We have multiple a uniform buffers for every frame in the swapchain.
	// We cycle them as we change the current swapchain image.
	// So vulkan will update the pointer and the engine will redirect it's camera write address
	void *newCameraBuffer;
	void *newSceneBuffer;
};

struct renderer_platform;
	
#define FUNC_RENDERER_INITIALIZE(name) bool name(renderer_platform *renderer, const char *appName)
typedef FUNC_RENDERER_INITIALIZE(func_renderer_initialize);

#define FUNC_RENDERER_DRAW_FRAME(name) bool name(render_packet *packet)
typedef FUNC_RENDERER_DRAW_FRAME(func_renderer_draw_frame);

#define FUNC_RENDERER_CHANGE_GRAPHICS_SETTINGS(name) bool name(graphics_settings settings, CHANGE_GRAPHICS_SETTINGS newSettings)
typedef FUNC_RENDERER_CHANGE_GRAPHICS_SETTINGS(func_renderer_change_graphics_settings);

#define FUNC_RENDERER_ON_SHADER_RELOAD(name) bool name()
typedef FUNC_RENDERER_ON_SHADER_RELOAD(func_renderer_on_shader_reload);

#define FUNC_RENDERER_ON_RESIZE(name) bool name(renderer_platform *renderer)
typedef FUNC_RENDERER_ON_RESIZE(func_renderer_on_resize);

#define FUNC_RENDERER_CREATE_SURFACE(name) void name(void *surfaceInfoIn)
typedef FUNC_RENDERER_CREATE_SURFACE(func_renderer_create_surface);

struct renderer_platform
{	
	RENDERER_GRAPHICS_API gfxAPI;
	bool canRender;
	u32 windowWidth;
	u32 windowHeight;
	
	func_renderer_initialize *Initialize;
	func_renderer_draw_frame *DrawFrame;
	func_renderer_change_graphics_settings *ChangeGraphicsSettings;
	func_renderer_on_shader_reload *OnShaderReload;
	func_renderer_on_resize *OnResize;
	
	func_renderer_create_surface *FillSurfaceWindowContext;
};

#endif // INCLUDE_RENDERER_PLATFORM_H