#ifndef HY3D_WIN32_PLATFORM_H
#define HY3D_WIN32_PLATFORM_H 1

#include "core.h"
#include "windows_configed.h"
#include "win32_resource.h"
#include "engine_platform.h"


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

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;
    
	bool isValid;
};

#endif
