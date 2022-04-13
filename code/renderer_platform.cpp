#include "renderer_platform.h"

#include "vulkan_platform.cpp"

static_func bool RendererSetGraphicsApiFunctions(RENDERER_GRAPHICS_API gfxAPI, engine_platform *engine)
{
	bool result = false;
	switch (gfxAPI)
	{
		case RENDERER_GRAPHICS_API_VULKAN:
		{
#if HY3D_DEBUG
			bool enoughGraphicsContextMem = sizeof(graphics_context) >= sizeof(vulkan_context);
			Assert(enoughGraphicsContextMem);
#endif
			vulkanContext = (vulkan_context *)&engine->state->graphicsContext;
			engine->renderer.Initialize = VulkanInitialize;
			engine->renderer.DrawFrame = VulkanDraw;
			engine->renderer.ChangeGraphicsSettings = VulkanChangeGraphicsSettings;
			engine->renderer.Upload = VulkanUpload;
			engine->renderer.BindShaderResources = VulkanBindShaderResources;
			engine->renderer.RequestBuffer = VulkanRequestBuffer;
			engine->renderer.OnShaderReload = VulkanOnShaderReload;
			engine->renderer.OnResize = VulkanOnResize;

			result = true;
		} break;

		default:
		{
			Assert(0);
		}
	}
	return result;
}

static_func bool RendererInitialize(RENDERER_GRAPHICS_API gfxAPI, engine_platform *engine)
{
	engine->renderer.windowWidth = engine->windowWidth;
	engine->renderer.windowHeight = engine->windowHeight;
	
	bool result = RendererSetGraphicsApiFunctions(gfxAPI, engine);
	if (result)
	{
		engine->renderer.gfxAPI = gfxAPI;
		engine->renderer.canRender = true;
		engine->renderer.Initialize(&engine->renderer, "HY3D ENGINE");
	}
	DebugPrintFunctionResult(result);
	return result;
}

static_func void RendererDestroy(renderer_platform *renderer)
{
	switch (renderer->gfxAPI)
	{
		case RENDERER_GRAPHICS_API_VULKAN:
		{
			VulkanDestroy();
		}
		break;
	}
}

static_func void RendererOnEngineReload(engine_platform *engine)
{
	switch (engine->renderer.gfxAPI)
	{
		case RENDERER_GRAPHICS_API_VULKAN:
		{
			vulkanContext = (vulkan_context *)&engine->state->graphicsContext;
			VulkanLoadCode();
			VulkanLoadGlobalFunctions();
			VulkanLoadInstanceFunctions();
			VulkanLoadDeviceFunctions();
			break;
		}
	}
	RendererSetGraphicsApiFunctions(engine->renderer.gfxAPI, engine);
}