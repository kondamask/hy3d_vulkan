#ifndef HY3D_WIN32_PLATFORM_H
#define HY3D_WIN32_PLATFORM_H 1

#include "windows_configed.h"
#include "core.h"
#include "engine_platform.h"
#include "win32_resource.h"

#define WIN32_WINDOW_Y_BORDER 39
#define WIN32_WINDOW_X_BORDER 23

#define VK_CODE unsigned char

struct wnd_dim
{
	u16 width, height;
};

struct win32_window
{
	wnd_dim dimensions;
	HWND handle;
	HINSTANCE instance;
	const char *name = "hy3d vulkan";
	bool onResize;
};
typedef win32_window window_context;

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;
    
	update_and_render *UpdateAndRender;
    
	bool isValid;
};

#endif
