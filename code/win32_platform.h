#pragma once
#include "hyv_base.h"
#include "hyv_windows.h"
#include "hyv_engine.h"
#include "hyv_vulkan.h"

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
	const char *name = "HYV_ENGINE";
	vulkan_state vulkan;
};

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;

	update_and_render *UpdateAndRender;

	bool isValid;
};
