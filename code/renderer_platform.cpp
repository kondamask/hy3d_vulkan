#include "renderer_platform.h"

#include "vulkan_platform.cpp"

static_func bool RendererInitialize(RENDERER_GRAPHICS_API gfxAPI, u32 windowWidth, u32 windowHeight, renderer_platform *outRenderer)
{
	outRenderer->windowWidth = windowWidth;
	outRenderer->windowHeight = windowHeight;
		
	bool result = true;
	switch (gfxAPI)
	{
		case RENDERER_GRAPHICS_API_VULKAN:
		{
			outRenderer->Initialize = VulkanInitialize;
			outRenderer->DrawFrame = VulkanDraw;
			outRenderer->ChangeGraphicsSettings = VulkanChangeGraphicsSettings;
			outRenderer->OnShaderReload = VulkanOnShaderReload; // CreatePipeline
			outRenderer->OnResize = VulkanOnResize; // CreateSwapchain
			result = true;
		} break;
		
		default:
		{
			Assert(0);
		}
	}
	
	if (result)
	{
		outRenderer->gfxAPI = gfxAPI;
		outRenderer->canRender = true;
		outRenderer->Initialize(outRenderer, "HY3D ENGINE");
	}
	
	return result;
}

static_func void RendererDestroy(renderer_platform *renderer)
{
	switch (renderer->gfxAPI)
	{
		case RENDERER_GRAPHICS_API_VULKAN:
		{
			VulkanDestroy();
		} break;
	}
}
