#ifndef INCLUDE_RENDERER_PLATFORM_H
#define INCLUDE_RENDERER_PLATFORM_H

#include "core.h"
#include "math.h"
#include "graphics_settings.h"
#include "engine_shader.h"

//------------------------------------------------------------------------

#define GRAPHICS_CONTEXT_SIZE KILOBYTES(10)

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

// NOTE: Some of these only apply on vulkan.
// I can probably just remap them in the background on other apis.
enum RENDERER_BUFFER_TYPE
{
	RENDERER_BUFFER_TYPE_UNIFORM,
	RENDERER_BUFFER_TYPE_UNIFORM_DYNAMIC,
	RENDERER_BUFFER_TYPE_STORAGE,
	RENDERER_BUFFER_TYPE_STORAGE_DYNAMIC,
	RENDERER_BUFFER_TYPE_STAGING,
};

struct render_packet
{
	// ENGINE -> RENDERER
	f32 dt;
	vec3 clearColor;
	vec3 playerPos;
	
	//------------------------------------------------------------------------
	
	// RENDERER -> ENGINE
	void *nextCameraPtr;
	void *nextScenePtr;
	void *nextTransformsPtr;
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

#define FUNC_RENDERER_UPLOAD_RESOURCES(name) bool name(staged_resources *staged)
typedef FUNC_RENDERER_UPLOAD_RESOURCES(func_renderer_upload_resources);

#define FUNC_RENDERER_REQUEST_BUFFER(name) void *name(RENDERER_BUFFER_TYPE bufferType, u64 size, bool doubleBuffer)
typedef FUNC_RENDERER_REQUEST_BUFFER(func_renderer_request_buffer);

#define FUNC_RENDERER_BIND_SHADER_RESOURCES(name) bool name(shader_resource_bind *binds, u32 count)
typedef FUNC_RENDERER_BIND_SHADER_RESOURCES(func_renderer_bind_shader_resources);

struct renderer_platform
{	
	RENDERER_GRAPHICS_API gfxAPI;
	bool canRender;
	u32 windowWidth;
	u32 windowHeight;
	
	//------------------------------------------------------------------------
	// API for Engine	
	func_renderer_initialize *Initialize;
	func_renderer_draw_frame *DrawFrame;
	func_renderer_change_graphics_settings *ChangeGraphicsSettings;
	func_renderer_upload_resources *Upload;
	func_renderer_request_buffer *RequestBuffer;
	func_renderer_bind_shader_resources *BindShaderResources;
	
	func_renderer_on_shader_reload *OnShaderReload;
	func_renderer_on_resize *OnResize;
	
	//------------------------------------------------------------------------
	// Platform Specific Called From The Graphics API
	func_renderer_create_surface *FillSurfaceWindowContext;
};

#endif // INCLUDE_RENDERER_PLATFORM_H