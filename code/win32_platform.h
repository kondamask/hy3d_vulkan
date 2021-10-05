#pragma once
#include "hyv_windows.h"
#include "hyv_engine.h"
#include "hyv_base.h"

#include <vulkan/vulkan.h>

#define VK_CODE unsigned char

struct wnd_dim
{
	u16 width, height;
};

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
struct win32_vulkan_state
{
	VkInstance instance;
	VkPhysicalDevice gpu;
	VkDevice device;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;

	VkQueue queue;

	VkCommandPool cmdPool;
	VkCommandBuffer cmdBuffer;

	VkSwapchainKHR swapchain;
	u32 swapchainImageCount;
	VkImage swapchainImages[2];
	VkImageView imageViews[2];

	VkImage depthImage;

#if VULKAN_VALIDATION_LAYERS_ON
	VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

struct win32_window
{
	wnd_dim dimensions;
	HWND handle;
	HINSTANCE instance;
	const char *name = "HYV_ENGINE";
	win32_vulkan_state vulkan;
};

struct win32_engine_code
{
	HMODULE dll;
	FILETIME writeTime;

	update_and_render *UpdateAndRender;

	bool isValid;
};