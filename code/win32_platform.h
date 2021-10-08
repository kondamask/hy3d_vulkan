#pragma once
#include "hy3d_base.h"
#include "hy3d_windows.h"
#include "hy3d_engine.h"

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

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;

	update_and_render *UpdateAndRender;

	bool isValid;
};
