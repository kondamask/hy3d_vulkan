#include "renderer_platform.h"

#include "vulkan_platform.cpp"

static_func bool RendererInitialize(RENDERER_GRAPHICS_API gfxAPI, engine_context *engine)
{
	renderer_platform *renderer = &engine->renderer;
	renderer->windowWidth = engine->windowWidth;
	renderer->windowHeight = engine->windowHeight;

	bool initialized = false;
	switch (gfxAPI)
	{
	case RENDERER_GRAPHICS_API_VULKAN:
	{
#if HY3D_DEBUG
		bool enoughGraphicsContextMem = sizeof(graphics_context) >= sizeof(vulkan_context);
		Assert(enoughGraphicsContextMem);
#endif
		vulkanContext = (vulkan_context *)&engine->state->graphicsContext;
		renderer->Initialize = VulkanInitialize;
		renderer->DrawFrame = VulkanDraw;
		renderer->ChangeGraphicsSettings = VulkanChangeGraphicsSettings;
		renderer->Upload = VulkanUpload;
		renderer->OnShaderReload = VulkanOnShaderReload;
		renderer->OnResize = VulkanOnResize;

		initialized = true;
	}
	break;

	default:
	{
		Assert(0);
	}
	}

	if (initialized)
	{
		renderer->gfxAPI = gfxAPI;
		renderer->canRender = true;
		renderer->Initialize(renderer, "HY3D ENGINE");
	}
	return initialized;
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

static_func void RendererOnEngineReload(engine_context *engine)
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
}