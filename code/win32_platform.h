#ifndef HY3D_WIN32_PLATFORM_H
#define HY3D_WIN32_PLATFORM_H 1

#include "windows_configed.h"
#include "win32_resource.h"

#include "core.h"

#define WIN32_WINDOW_Y_BORDER 39
#define WIN32_WINDOW_X_BORDER 23

#define VK_CODE unsigned char

struct win32_window
{
	u32 width, height;
	HWND handle;
	HINSTANCE instance;
	const char *name = "hy3d vulkan";
};
typedef win32_window os_window_context;
global_var os_window_context osWindowContext;

#include "engine_platform.h"

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;
    
	func_engine_initialize *Initialize;
	func_engine_update_and_render *UpdateAndRender;
	func_engine_destroy *Destroy;
    
	bool isValid;
};

#endif
