#include "win32_platform.h"
#include "vulkan_platform.h"

// TODO: Find some way to contain this in the vulkan_platform
// For now I don't know how to get the hInstance and window handle from windows.
FUNC_RENDERER_CREATE_SURFACE(Win32VulkanFillSurfaceWindowContext)
{
	VkWin32SurfaceCreateInfoKHR *surfaceInfo = (VkWin32SurfaceCreateInfoKHR *)surfaceInfoIn;
	surfaceInfo->hinstance = osWindowContext.instance;
	surfaceInfo->hwnd = osWindowContext.handle;
}