#pragma once
#include "hy3d_windows.h"
#include "hy3d_engine.h"
#include <vulkan/vulkan.h>

#define VK_CODE unsigned char

struct win32_window_dimensions
{
	i16 width, height;
};

struct win32_pixel_buffer
{
	BITMAPINFO info;
	void *memory;
	void *zBuffer;
	i16 width;
	i16 height;
	i32 size;
	i8 bytesPerPixel;
};

struct win32_window
{
	win32_pixel_buffer pixelBuffer;
	win32_window_dimensions dimensions;
	HWND handle;
	HINSTANCE instance;
	LPCSTR className = "HY3D_WINDOW_CLASS";
};

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;

	update_and_render *UpdateAndRender;

	bool isValid;
};