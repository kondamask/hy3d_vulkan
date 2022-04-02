#include "hy3d_vulkan.h"

namespace Vulkan
{
	static_func bool Win32LoadDLL();
	static_func bool Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name);

	static_func bool CreateRenderPass();
	static_func bool CreateFrameBuffers();
	static_func bool CreateSwapchain();
	static_func bool CreatePipeline();
	static_func bool CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, vulkan_buffer &buffer, bool mapBuffer = false);
	static_func bool CreateImage(VkImageType type, VkFormat format, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectMask, vulkan_image &imageOut);

	static_func bool CreateDepthBuffer();
	static_func bool CreateMSAABuffer();

	static_func void ClearRenderPass();
	static_func void ClearFrameBuffers();
	static_func void ClearSwapchainImages();
	static_func void ClearImage(vulkan_image &img);
	static_func void ClearPipeline();
	static_func void ClearBuffer(vulkan_buffer buffer);
	static_func void ClearSwapchain();
	static_func void Destroy();

	static_func bool ChangeGraphicsSettings(graphics_settings settings, CHANGE_GRAPHICS_SETTINGS newSettings);

	static_func bool Draw(update_data *data);

	static_func bool PushStaged(staged_resources &stagedResources);
	static_func bool LoadShader(char *filepath, VkShaderModule *shaderOut);

	static_func void PickMSAA(MSAA_OPTIONS msaa);
	static_func bool FindMemoryProperties(u32 memoryType, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndexOut);
	static_func u64 GetUniformBufferPaddedSize(u64 originalSize);

	static_func void CmdChangeImageLayout(VkCommandBuffer cmdBuffer, VkImage imgHandle, image *imageInfo, VkImageLayout oldLayout, VkImageLayout newLayout);

	static_func frame_prep_resource *GetNextAvailableResource();
	/*
    static_func void GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc);
    static_func void GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs);
    */
#if VULKAN_VALIDATION_LAYERS_ON
	static_func VKAPI_ATTR VkBool32 VKAPI_CALL
	DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
		void *pUserData);
#endif
}

#if VULKAN_VALIDATION_LAYERS_ON
static_func VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
	void *pUserData)
{
	bool isError = false;
	char *type;
	if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
	type = "Some general event has occurred";
	else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	type = "Something has occurred during validation against the Vulkan specification that may indicate invalid behavior.";
	else
	type = "Potentially non-optimal use of Vulkan.";

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		DebugPrint("WARNING: ");
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		DebugPrint("DIAGNOSTIC: ");
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		DebugPrint("INFO: ");
	}
	else
	{
		DebugPrint("ERROR: ");
		isError = true;
	}
	DebugPrint(type);
	DebugPrint('\n');
	DebugPrint(callbackData->pMessageIdName);
	DebugPrint('\n');

	char c;
	u32 i = 0;
	do
	{
		c = callbackData->pMessage[i];
		i++;
	} while (c != ']');
	i++;
	do
	{
		c = callbackData->pMessage[i];
		if (c != ';' && c != '|')
		{
			DebugPrint(c);
			i++;
		}
		else
		{
			DebugPrint('\n');
			i++;
			do
			{
				c = callbackData->pMessage[i];
				i++;
			} while (c == ' ' || c == '|');
			i--;
		}
	} while (c != '\0');
	DebugPrint('\n');
	if (isError)
	{
		Assert(0);
		return VK_FALSE;
	}
	return VK_FALSE;
}

#endif

static_func void Vulkan::PickMSAA(MSAA_OPTIONS msaa)
{
	VkSampleCountFlags counts = vulkan.gpuProperties.limits.framebufferColorSampleCounts & vulkan.gpuProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT && msaa >= MSAA_64)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT && msaa >= MSAA_32)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT && msaa >= MSAA_16)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT && msaa >= MSAA_8)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT && msaa >= MSAA_4)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT && msaa >= MSAA_2)
		vulkan.msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else
		vulkan.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

static_func bool Vulkan::FindMemoryProperties(u32 reqMemType, VkMemoryPropertyFlags reqMemProperties, u32 &memoryIndexOut)
{
	// NOTE(heyyod): We assume we have already set the memory properties during creation.
	u32 memoryCount = vulkan.memoryProperties.memoryTypeCount;
	for (memoryIndexOut = 0; memoryIndexOut < memoryCount; ++memoryIndexOut)
	{
		uint32_t memoryType = (1 << memoryIndexOut);
		bool isRequiredMemoryType = reqMemType & memoryType;
		if (isRequiredMemoryType)
		{
			VkMemoryPropertyFlags properties = vulkan.memoryProperties.memoryTypes[memoryIndexOut].propertyFlags;
			bool hasRequiredProperties = ((properties & reqMemProperties) == reqMemProperties);
			if (hasRequiredProperties)
				return true;
		}
	}
	return false;
}

static_func u64 Vulkan::GetUniformBufferPaddedSize(u64 originalSize)
{
	u64 minUboAlignment = vulkan.gpuProperties.limits.minUniformBufferOffsetAlignment;
	u64 alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

// TODO: make this cross-platform
static_func bool Vulkan::Win32LoadDLL()
{
	vulkan.dll = LoadLibraryA("vulkan-1.dll");
	if (!vulkan.dll)
		return false;

	vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkan.dll, "vkGetInstanceProcAddr");

	DebugPrint("Loaded DLL\n");
	return true;
}


// TODO: Make it cross-platform
static_func bool Vulkan::Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name)
{
	DebugPrint("Initialize Vulkan\n");
	if (!Win32LoadDLL())
	{
		return false;
	}

	if (!VulkanLoadGlobalFunctions())
	{
		return false;
	}

	// NOTE: Create an instance
	{
#if VULKAN_VALIDATION_LAYERS_ON
		char *instanceLayers[] = {
			"VK_LAYER_KHRONOS_validation"
		};
#else
		/* NOTE(heyyod): Normaly:
        char *instanceLayers[] = {
...
};
            */
		char **instanceLayers = 0;
#endif
		if (instanceLayers)
		{
			u32 layerCount;
			AssertSuccess(vkEnumerateInstanceLayerProperties(&layerCount, 0));

			VkLayerProperties availableLayers[32];
			Assert(layerCount <= ArrayCount(availableLayers));
			AssertSuccess(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

			for (u32 i = 0; i < ArrayCount(instanceLayers); i++)
			{
				char *layerName = instanceLayers[i];
				bool layerFound = false;
				for (VkLayerProperties &layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}
				if (!layerFound)
				{
					Assert("ERROR: Layer not found");
					return false;
				}
			}
		}
		else
		{

		}

#if VULKAN_VALIDATION_LAYERS_ON
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
		debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		;
		debugMessengerInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		;
		debugMessengerInfo.pfnUserCallback = DebugCallback;
		DebugPrint("Created Vulkan Debug Messenger\n");
#endif

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = name;
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		char *desiredInstanceExtensions[] = {
#if VULKAN_VALIDATION_LAYERS_ON
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
				VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
		};
		u32 instanceExtensionsCount;
		VkExtensionProperties availableInstanceExtensions[255] = {};
		AssertSuccess(vkEnumerateInstanceExtensionProperties(0, &instanceExtensionsCount, 0));
		Assert(instanceExtensionsCount <= ArrayCount(availableInstanceExtensions));
		AssertSuccess(vkEnumerateInstanceExtensionProperties(0, &instanceExtensionsCount, availableInstanceExtensions));
		for (char *desiredInstanceExtension : desiredInstanceExtensions)
		{
			bool found = false;
			for (VkExtensionProperties &availableExtension : availableInstanceExtensions)
			{
				if (strcmp(desiredInstanceExtension, availableExtension.extensionName) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				Assert("ERROR: Requested instance extension not supported");
				return false;
			}
		}

		// NOTE: Create an instance and attach extensions
		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = ArrayCount(desiredInstanceExtensions);
		instanceInfo.ppEnabledExtensionNames = desiredInstanceExtensions;
		if (instanceLayers)
		{
			instanceInfo.enabledLayerCount = ArrayCount(instanceLayers);
			instanceInfo.ppEnabledLayerNames = instanceLayers;
		}
#if VULKAN_VALIDATION_LAYERS_ON
		instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif

		AssertSuccess(vkCreateInstance(&instanceInfo, 0, &vulkan.instance));
		DebugPrint("Created Vulkan Instance\n");

		if (!VulkanLoadInstanceFunctions())
		{
			Assert("ERROR: Instance Functions not loaded\n");
			return false;
		}

#if VULKAN_VALIDATION_LAYERS_ON
		AssertSuccess(vkCreateDebugUtilsMessengerEXT(vulkan.instance, &debugMessengerInfo, 0, &vulkan.debugMessenger));
#endif
	}

	// NOTE: Create a surface
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = wndInstance;
		surfaceInfo.hwnd = wndHandle;
		AssertSuccess(vkCreateWin32SurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));

#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.connection = 0; //we'll have these if we implement a linux window
		surfaceInfo.window = 0;     //we'll have these if we implement a linux window
		AssertSuccess(vkCreateXcbSurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.dpy = 0;    //we'll have these if we implement a mac(?) window
		surfaceInfo.window = 0; //we'll have these if we implement a mac(?) window
		AssertSuccess(vkCreateXlibSurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));
#endif
	}
	// NOTE: Select a gpu to use
	{
		u32 gpuCount = 0;
		VkPhysicalDevice gpuBuffer[16] = {};
		AssertSuccess(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, 0));
		Assert(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
		AssertSuccess(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, gpuBuffer));
		vulkan.gpu = gpuBuffer[0];
		// TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
		//For now it's ok since I only have 1 gpu.

		vkGetPhysicalDeviceProperties(vulkan.gpu, &vulkan.gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(vulkan.gpu, &vulkan.memoryProperties);

		// NOTE(heyyod): Pick number of samples to use for antialiasig
		PickMSAA(START_UP_MSAA);

		DebugPrint("Selected a GPU\n");
	}

	// NOTE: Pick a queue family the supports both present and graphics operations
	vulkan.graphicsQueueFamilyIndex = UINT32_MAX;
	vulkan.presentQueueFamilyIndex = UINT32_MAX;
	vulkan.transferQueueFamilyIndex = UINT32_MAX;
	{
		u32 queueFamilyCount;
		VkQueueFamilyProperties availableQueueFamilies[16] = {};
		VkBool32 supportsPresent[16] = {};
		vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, 0);
		Assert(queueFamilyCount <= ArrayCount(availableQueueFamilies));
		vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, availableQueueFamilies);

		for (u32 i = 0; i < queueFamilyCount; i++)
			AssertSuccess(vkGetPhysicalDeviceSurfaceSupportKHR(vulkan.gpu, i, vulkan.surface, &supportsPresent[i]));

		for (u32 i = 0; i < queueFamilyCount; ++i)
		{
			// NOTE(heyyod): Find a graphics queue
			if ((availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (vulkan.graphicsQueueFamilyIndex == UINT32_MAX)
					vulkan.graphicsQueueFamilyIndex = i;
				if (supportsPresent[i] == VK_TRUE)
				{
					vulkan.graphicsQueueFamilyIndex = i;
					vulkan.presentQueueFamilyIndex = i;
					break;
				}
			}
		}
		for (u32 i = 0; i < queueFamilyCount; ++i)
		{
			// NOTE(heyyod): Find a transfer queue
			if ((availableQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				if (vulkan.transferQueueFamilyIndex == UINT32_MAX)
					vulkan.transferQueueFamilyIndex = i;
			}
		}
		if (vulkan.presentQueueFamilyIndex == UINT32_MAX) //didn't find a queue that supports both graphics and present
		{
			for (u32 i = 0; i < queueFamilyCount; ++i)
				if (supportsPresent[i] == VK_TRUE)
				{
					vulkan.presentQueueFamilyIndex = i;
					break;
				}
		}
		if (vulkan.graphicsQueueFamilyIndex == UINT32_MAX)
		{
			Assert("ERROR: No graphics queue found.\n");
			return false;
		}
		if (vulkan.presentQueueFamilyIndex == UINT32_MAX)
		{
			Assert("ERROR: No present queue found.\n");
			return false;
		}
	}

	// TODO: make this work for seperate queues if needed
	if (vulkan.graphicsQueueFamilyIndex != vulkan.presentQueueFamilyIndex)
	{
		Assert("vulkan.graphicsQueueFamilyIndex != vulkan.presentQueueFamilyIndex\n");
		return false;
	}
	DebugPrint("Selected a queue family\n");

	// TODO: Need to do some stuff if they are different like:
	//VkDeviceQueueCreateInfo queueInfo[2] = {};
	//float queuePriority = 1.0; // must be array of size queueCount
	//
	//queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//queueInfo[0].queueFamilyIndex = graphicsQueueFamilyIndex;
	//queueInfo[0].queueCount = 1;
	//queueInfo[0].pQueuePriorities = &queuePriority;
	//
	//queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//queueInfo[1].queueFamilyIndex = presentQueueFamilyIndex;
	//queueInfo[1].queueCount = 1;
	//queueInfo[1].pQueuePriorities = &queuePriority;

	// NOTE: Create a device
	VkDeviceQueueCreateInfo queueInfo = {};
	{
		float queuePriority = 1.0; // must be array of size queueCount
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = vulkan.graphicsQueueFamilyIndex;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;

		char *desiredDeviceExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		u32 deviceExtensionsCount;
		VkExtensionProperties availableDeviceExtensions[255] = {};
		AssertSuccess(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, 0));
		Assert(deviceExtensionsCount <= ArrayCount(availableDeviceExtensions));
		AssertSuccess(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, availableDeviceExtensions));
		for (char *desiredDeviceExtension : desiredDeviceExtensions)
		{
			bool found = false;
			for (VkExtensionProperties &availableExtension : availableDeviceExtensions)
			{
				if (strcmp(desiredDeviceExtension, availableExtension.extensionName) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				Assert("ERROR: Requested device extension not supported");
				return false;
			}
		}

		// TODO(heyyod): Check if these features are supported;
		VkPhysicalDeviceFeatures desiredFeatures = {};
		desiredFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.enabledExtensionCount = ArrayCount(desiredDeviceExtensions);
		deviceInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
		deviceInfo.pEnabledFeatures = &desiredFeatures;
		AssertSuccess(vkCreateDevice(vulkan.gpu, &deviceInfo, 0, &vulkan.device));
		DebugPrint("\nCreated a Vulkan Device\n");

		if (!VulkanLoadDeviceFunctions())
		{
			Assert("ERROR: Device Functions not loaded\n");
			return false;
		}
	}

	// NOTE: Get the queues and save it into our vulkan object
	vkGetDeviceQueue(vulkan.device, vulkan.graphicsQueueFamilyIndex, 0, &vulkan.graphicsQueue);
	vkGetDeviceQueue(vulkan.device, vulkan.presentQueueFamilyIndex, 0, &vulkan.presentQueue);

	// NOTE: Set a proper surface format
	{
		vulkan.surfaceFormat.colorSpace = SURFACE_FORMAT_COLOR_SPACE;
		vulkan.surfaceFormat.format = SURFACE_FORMAT_FORMAT;
		u32 formatCount = 0;
		VkSurfaceFormatKHR availableFormats[16] = {};
		bool desiredSurfaceFormatSupported = false;
		AssertSuccess(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, 0));
		Assert(formatCount <= ArrayCount(availableFormats));
		for (u32 i = 0; i < formatCount; ++i)
			AssertSuccess(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, &availableFormats[i]));
		for (u32 i = 0; i < formatCount; ++i)
		{
			if (vulkan.surfaceFormat.format == availableFormats[i].format &&
				vulkan.surfaceFormat.colorSpace == availableFormats[i].colorSpace)
				desiredSurfaceFormatSupported = true;
		}
		Assert(desiredSurfaceFormatSupported);
	}

	// NOTE(heyyod): Create the rendering resources
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = vulkan.presentQueueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		AssertSuccess(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));

		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


		for (u32 i = 0; i < NUM_RESOURCES; i++)
		{
			AssertSuccess(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, &vulkan.resources[i].cmdBuffer));
			AssertSuccess(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.resources[i].imgAvailableSem));
			AssertSuccess(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.resources[i].frameReadySem));
			AssertSuccess(vkCreateFence(vulkan.device, &fenceInfo, 0, &vulkan.resources[i].fence));
		}
		DebugPrint("Created rendering resources.\n");
	}

	// NOTE(heyyod): Make buffers
	{
		u32 vertexBufferSize = MEGABYTES(128);
		u32 indexBufferSize = MEGABYTES(128);
		u32 stagingBufferSize = vertexBufferSize + indexBufferSize;
		if (!CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vulkan.stagingBuffer, MAP_BUFFER_TRUE))
		{
			DebugPrint("Could not create index buffer\n");
			Assert("Could not create staging buffer");
			return false;
		}

		if (!CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan.vertexBuffer))
		{
			DebugPrint("Could not create vertex buffer\n");
			Assert("Could not create vertex buffer");
			return false;
		}

		if (!CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan.indexBuffer))
		{
			DebugPrint("Could not create index buffer\n");
			Assert("Could not create index buffer");
			return false;
		}
	}


	// NOTE(heyyod): Create texture sampler
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = vulkan.gpuProperties.limits.maxSamplerAnisotropy;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 11.0f; // TODO(heyyod): THIS IS HARDCODED AND BAD
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		AssertSuccess(vkCreateSampler(vulkan.device, &samplerInfo, 0, &vulkan.textureSampler));
	}

	// NOTE(heyyod): Create static transforms buffer
	if (!CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, MAX_RENDER_OBJECTS * sizeof(object_transform),
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vulkan.staticTransformsBuffer, MAP_BUFFER_TRUE))
	{
		DebugPrint("Could not create static transforms storage buffer\n");
		Assert("Could not create static transforms storage buffer");
		return false;
	}

	// NOTE(heyyod): Create descriptors
	{
		// NOTE(heyyod): layout
		{
			VkDescriptorSetLayoutBinding bindings[] = {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, 0 }, // camera
				{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, 0 }, // objects
				{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, 0 }, //texture sampler
				{ 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0 }, //scene
			};

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = ArrayCount(bindings);
			layoutInfo.pBindings = bindings;
			AssertSuccess(vkCreateDescriptorSetLayout(vulkan.device, &layoutInfo, 0, &vulkan.globalDescSetLayout));
		}

		u32 nDescriptors = ArrayCount(vulkan.frameData);
		// NOTE(heyyod): pool
		{
			VkDescriptorPoolSize poolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nDescriptors },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nDescriptors },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nDescriptors },
			};

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = ArrayCount(poolSizes);
			poolInfo.pPoolSizes = poolSizes;
			poolInfo.maxSets = nDescriptors;
			AssertSuccess(vkCreateDescriptorPool(vulkan.device, &poolInfo, 0, &vulkan.globalDescPool));
		}

		// NOTE(heyyod): Allocate and write the descriptor sets
		VkDescriptorSetAllocateInfo globalDescAlloc = {};
		globalDescAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		globalDescAlloc.descriptorPool = vulkan.globalDescPool;
		globalDescAlloc.descriptorSetCount = 1;
		globalDescAlloc.pSetLayouts = &vulkan.globalDescSetLayout;
		for (u8 i = 0; i < nDescriptors; i++)
		{
			AssertSuccess(vkAllocateDescriptorSets(vulkan.device, &globalDescAlloc, &vulkan.frameData[i].globalDescriptor));

			// NOTE(heyyod): Camera buffer and descriptor
			VkDescriptorBufferInfo cameraBufferInfo = {};
			VkWriteDescriptorSet writeCameraDesc = {};
			if (CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(camera_data),
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				vulkan.frameData[i].cameraBuffer, MAP_BUFFER_TRUE))
			{
				cameraBufferInfo.range = sizeof(camera_data);
				cameraBufferInfo.buffer = vulkan.frameData[i].cameraBuffer.handle;

				writeCameraDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeCameraDesc.dstBinding = 0;
				writeCameraDesc.dstArrayElement = 0;
				writeCameraDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeCameraDesc.descriptorCount = 1;
				writeCameraDesc.pBufferInfo = &cameraBufferInfo;
				writeCameraDesc.dstSet = vulkan.frameData[i].globalDescriptor;
			}
			else
			{
				Assert("Could not create camera uniform buffers");
				return false;
			}

			// NOTE(heyyod): Storage buffer and descriptor
			VkDescriptorBufferInfo objectsBufferInfo;
			VkWriteDescriptorSet writeObjectsDesc = {};
			{
				objectsBufferInfo.range = sizeof(object_transform) * MAX_RENDER_OBJECTS;
				objectsBufferInfo.buffer = vulkan.staticTransformsBuffer.handle;

				writeObjectsDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeObjectsDesc.dstBinding = 1;
				writeObjectsDesc.dstArrayElement = 0;
				writeObjectsDesc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeObjectsDesc.descriptorCount = 1;
				writeObjectsDesc.pBufferInfo = &objectsBufferInfo;
				writeObjectsDesc.dstSet = vulkan.frameData[i].globalDescriptor;
			}

			VkDescriptorBufferInfo sceneBufferInfo = {};
			VkWriteDescriptorSet writeSceneDesc = {};
			if (CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(scene_data),
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				vulkan.frameData[i].sceneBuffer, MAP_BUFFER_TRUE))
			{
				sceneBufferInfo.range = sizeof(scene_data);
				sceneBufferInfo.buffer = vulkan.frameData[i].sceneBuffer.handle;

				writeSceneDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSceneDesc.dstBinding = 3;
				writeSceneDesc.dstArrayElement = 0;
				writeSceneDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeSceneDesc.descriptorCount = 1;
				writeSceneDesc.pBufferInfo = &sceneBufferInfo;
				writeSceneDesc.dstSet = vulkan.frameData[i].globalDescriptor;
			}
			else
			{
				Assert("Could not create scene uniform buffer");
				return false;
			}

			VkWriteDescriptorSet writes[] = {
				writeCameraDesc,
				writeObjectsDesc,
				writeSceneDesc
			};
			vkUpdateDescriptorSets(vulkan.device, ArrayCount(writes), writes, 0, 0);
		}
	}

	if (!CreateRenderPass() || !CreatePipeline())
	{
		Assert("Could not create renderpass or pipeline");
		return false;
	}

	// NOTE(heyyod): It looks like window always sends a WM_SIZE message
	// at startup. This Recreats the following. So we don't need to create these here
	/* 
        if (!CreateSwapchain())
        {
            Assert("Failed to create swapchain");
            return false;
        }
        
        if (!CreateFrameBuffers())
        {
            Assert("Failed to create framebuffers");
            return false;
        }
         */

	vulkan.canRender = true;
	return true;
}

static_func void Vulkan::ClearRenderPass()
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);
		if (VulkanIsValidHandle(vulkan.renderPass))
			vkDestroyRenderPass(vulkan.device, vulkan.renderPass, 0);
	}
}

static_func bool Vulkan::CreateRenderPass()
{
	ClearRenderPass();

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.flags = 0;
	colorAttachment.format = vulkan.surfaceFormat.format;
	colorAttachment.samples = vulkan.msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAttachmentReference colorAttachementRef = {};
	colorAttachementRef.attachment = 0;
	colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = DEPTH_BUFFER_FORMAT;
	depthAttachment.samples = vulkan.msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// NOTE(heyyod): Specify the subpasses
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// NOTE(heyyod): The index of the color attachment in this array is directly 
	//referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
	subpass.colorAttachmentCount = 1; // ArrayCount(colorAttachementRef)
	subpass.pColorAttachments = &colorAttachementRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	//subpass.inputAttachmentCount = 0;
	//subpass.pInputAttachments = 0;
	//subpass.preserveAttachmentCount = 0;
	//subpass.pPreserveAttachments = 0;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vulkan.msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
		renderPassInfo.attachmentCount = ArrayCount(attachments);
		renderPassInfo.pAttachments = attachments;
		AssertSuccess(vkCreateRenderPass(vulkan.device, &renderPassInfo, 0, &vulkan.renderPass));
	}
	else
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// NOTE(heyyod): attachment to resolve the multisamplerd image into a presentable image
		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = vulkan.surfaceFormat.format;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment, colorAttachmentResolve };
		renderPassInfo.attachmentCount = ArrayCount(attachments);
		renderPassInfo.pAttachments = attachments;
		AssertSuccess(vkCreateRenderPass(vulkan.device, &renderPassInfo, 0, &vulkan.renderPass));
	}
	DebugPrint("Created a renderpass\n");

	return true;
}

static_func void Vulkan::ClearFrameBuffers()
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);
		for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
		{
			if (VulkanIsValidHandle(vulkan.framebuffers[i]))
			{
				vkDestroyFramebuffer(vulkan.device, vulkan.framebuffers[i], 0);
				//vulkan.framebuffers[i] = VK_NULL_HANDLE;
				DebugPrint("Cleared FrameBuffer\n");
			}
		}
	}
}

static_func bool Vulkan::CreateFrameBuffers()
{
	ClearFrameBuffers();
	local_var VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = vulkan.renderPass;
	framebufferInfo.width = vulkan.windowExtent.width;
	framebufferInfo.height = vulkan.windowExtent.height;
	framebufferInfo.layers = 1;
	for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
	{
		if (vulkan.msaaSamples == VK_SAMPLE_COUNT_1_BIT)
		{
			VkImageView attachments[] = { vulkan.swapchainImageViews[i], vulkan.depthBuffer.view };
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.attachmentCount = ArrayCount(attachments);
			AssertSuccess(vkCreateFramebuffer(vulkan.device, &framebufferInfo, 0, &vulkan.framebuffers[i]));
		}
		else
		{
			VkImageView attachments[] = { vulkan.msaa.view, vulkan.depthBuffer.view, vulkan.swapchainImageViews[i] };
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.attachmentCount = ArrayCount(attachments);
			AssertSuccess(vkCreateFramebuffer(vulkan.device, &framebufferInfo, 0, &vulkan.framebuffers[i]));
		}
	}
	return true;
}

static_func void Vulkan::ClearImage(vulkan_image &img)
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);
		if (VulkanIsValidHandle(img.view))
			vkDestroyImageView(vulkan.device, img.view, 0);
		if (VulkanIsValidHandle(img.handle))
			vkDestroyImage(vulkan.device, img.handle, 0);
		if (VulkanIsValidHandle(img.memoryHandle))
			vkFreeMemory(vulkan.device, img.memoryHandle, 0);

		img.view = VK_NULL_HANDLE;
		img.handle = VK_NULL_HANDLE;
		img.memoryHandle = VK_NULL_HANDLE;
	}
}

static_func bool Vulkan::CreateImage(VkImageType type, VkFormat format, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectMask, vulkan_image &imageOut)
{
	ClearImage(imageOut);

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = type;
	imageInfo.format = format;
	imageInfo.extent = extent;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = samples;
	imageInfo.tiling = tiling;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	AssertSuccess(vkCreateImage(vulkan.device, &imageInfo, 0, &imageOut.handle));

	VkMemoryRequirements memoryReq = {};
	u32 memIndex = 0;
	vkGetImageMemoryRequirements(vulkan.device, imageOut.handle, &memoryReq);
	if (Vulkan::FindMemoryProperties(memoryReq.memoryTypeBits, memoryProperties, memIndex))
	{
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryReq.size;
		allocInfo.memoryTypeIndex = memIndex;

		AssertSuccess(vkAllocateMemory(vulkan.device, &allocInfo, 0, &imageOut.memoryHandle));
		AssertSuccess(vkBindImageMemory(vulkan.device, imageOut.handle, imageOut.memoryHandle, 0));
	}
	else
	{
		Assert(0);
		return false;
	}

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = imageOut.handle;
	imageViewInfo.format = format;

	if (imageInfo.imageType == VK_IMAGE_TYPE_1D)
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
	else if (imageInfo.imageType == VK_IMAGE_TYPE_2D)
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	if (imageInfo.imageType == VK_IMAGE_TYPE_3D)
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;

	imageViewInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	imageViewInfo.subresourceRange = {
		aspectMask,         //VkImageAspectFlags    aspectMask;
		0,                  //uint32_t              baseMipLevel;
		mipLevels, //uint32_t              levelCount;
		0,                  //uint32_t              baseArrayLayer;
		1                   //uint32_t              layerCount;
	};
	AssertSuccess(vkCreateImageView(vulkan.device, &imageViewInfo, 0, &imageOut.view));

	return true;

	// NOTE(heyyod): This is a way to find a good format that is supported an proper for what we want
	/*
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
    for (VkFormat format : candidates)
    {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
    
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
    {
    return format;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
    {
    return format;
                }
            }
        }
    */
}

static_func bool Vulkan::CreateDepthBuffer()
{
	return CreateImage(VK_IMAGE_TYPE_2D, DEPTH_BUFFER_FORMAT,
		{ vulkan.windowExtent.width, vulkan.windowExtent.height, 1 }, 1, vulkan.msaaSamples,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, vulkan.depthBuffer);
}

static_func bool Vulkan::CreateMSAABuffer()
{
	if (vulkan.msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		ClearImage(vulkan.msaa);
		return true;
	}
	return CreateImage(VK_IMAGE_TYPE_2D, vulkan.surfaceFormat.format, { vulkan.windowExtent.width, vulkan.windowExtent.height, 1 }, 1, vulkan.msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, vulkan.msaa);
}

static_func bool Vulkan::CreateSwapchain()
{
	if (VulkanIsValidHandle(vulkan.device))
		vkDeviceWaitIdle(vulkan.device);
	else
		return false;

	vulkan.canRender = false;

	// NOTE: Get Surface capabilities
	{
		VkSurfaceCapabilitiesKHR surfCapabilities = {};
		AssertSuccess(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.gpu, vulkan.surface, &surfCapabilities));

		vulkan.windowExtent = {};
		if (surfCapabilities.currentExtent.width == UINT32_MAX)
		{
			vulkan.windowExtent.width = WINDOW_WIDTH;
			vulkan.windowExtent.height = WINDOW_HEIGHT;

			if (vulkan.windowExtent.width < surfCapabilities.minImageExtent.width)
				vulkan.windowExtent.width = surfCapabilities.minImageExtent.width;
			else if (vulkan.windowExtent.width > surfCapabilities.maxImageExtent.width)
				vulkan.windowExtent.width = surfCapabilities.maxImageExtent.width;

			if (vulkan.windowExtent.height < surfCapabilities.minImageExtent.height)
				vulkan.windowExtent.height = surfCapabilities.minImageExtent.height;
			else if (vulkan.windowExtent.height > surfCapabilities.maxImageExtent.height)
				vulkan.windowExtent.height = surfCapabilities.maxImageExtent.height;
		}
		else
		{ // If the surface size is defined, the swap chain size must match
			vulkan.windowExtent = surfCapabilities.currentExtent;
		}
		if ((vulkan.windowExtent.width == 0) || (vulkan.windowExtent.height == 0))
		{
			return true;
		}

		// NOTE: Determine the number of VkImage's to use in the swap chain.
		//Constant at 2 for now: Double buffering
		Assert(NUM_SWAPCHAIN_IMAGES >= surfCapabilities.minImageCount);
		Assert(NUM_SWAPCHAIN_IMAGES <= surfCapabilities.maxImageCount);
		vulkan.swapchainImageCount = NUM_SWAPCHAIN_IMAGES;

		// NOTE: Determine the pre-transform
		VkSurfaceTransformFlagBitsKHR desiredPreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //do nothing
		if (!(surfCapabilities.supportedTransforms & desiredPreTransform))
			desiredPreTransform = surfCapabilities.currentTransform;

		// NOTE: Set the present mode
		VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		//The FIFO present mode is guaranteed by the spec to be supported

		uint32_t presentModeCount;
		AssertSuccess(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, NULL));
		VkPresentModeKHR presentModes[16] = {};;
		Assert(presentModeCount <= ArrayCount(presentModes));
		AssertSuccess(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, presentModes));
		bool desiredPresentModeSupported = false;
		for (u32 i = 0; i < presentModeCount; i++)
		{
			if (desiredPresentMode == presentModes[i])
				desiredPresentModeSupported = true;
		}
		if (!desiredPresentModeSupported)
		{
			desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;//VK_PRESENT_MODE_IMMEDIATE_KHR;
		}

		// NOTE: Find a supported composite alpha mode - one of these is guaranteed to be set
		VkCompositeAlphaFlagBitsKHR desiredCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (uint32_t i = 0; i < ArrayCount(compositeAlphaFlags); i++)
		{
			if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
			{
				desiredCompositeAlpha = compositeAlphaFlags[i];
				break;
			}
		}

		// NOTE: Set image usage flags
		VkImageUsageFlags desiredImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //must alwasy be supported
		if (surfCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			desiredImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		else
		{
			DebugPrint("VK_IMAGE_USAGE_TRANSFER_DST_BIT not supported\n");
			return false;
		}

		// NOTE: Make the actual swapchain
		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = vulkan.surface;
		swapchainInfo.imageExtent = vulkan.windowExtent;
		swapchainInfo.imageFormat = vulkan.surfaceFormat.format;
		swapchainInfo.imageColorSpace = vulkan.surfaceFormat.colorSpace;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = desiredImageUsageFlags;
		swapchainInfo.minImageCount = vulkan.swapchainImageCount;
		swapchainInfo.preTransform = desiredPreTransform;
		swapchainInfo.compositeAlpha = desiredCompositeAlpha;
		swapchainInfo.presentMode = desiredPresentMode;
		swapchainInfo.clipped = true;

		u32 queueFamilyIndices[2] = { vulkan.graphicsQueueFamilyIndex, vulkan.presentQueueFamilyIndex };
		if (vulkan.graphicsQueueFamilyIndex != vulkan.presentQueueFamilyIndex)
		{
			// If the graphics and present queues are from different queue families,
			// we either have to explicitly transfer ownership of images between
			// the queues, or we have to create the swapchain with imageSharingMode
			// as VK_SHARING_MODE_CONCURRENT
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VkSwapchainKHR oldSwapchain = vulkan.swapchain;
		swapchainInfo.oldSwapchain = oldSwapchain;
		AssertSuccess(vkCreateSwapchainKHR(vulkan.device, &swapchainInfo, 0, &vulkan.swapchain));
		if (VulkanIsValidHandle(oldSwapchain)) //old swapchain
			vkDestroySwapchainKHR(vulkan.device, oldSwapchain, 0);
	}

	// NOTE: Create the Image views
	{
		ClearSwapchainImages();
		AssertSuccess(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, 0));
		Assert(vulkan.swapchainImageCount == ArrayCount(vulkan.swapchainImages));
		AssertSuccess(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, vulkan.swapchainImages));

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = vulkan.surfaceFormat.format;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
		{
			imageViewInfo.image = vulkan.swapchainImages[i];
			AssertSuccess(vkCreateImageView(vulkan.device, &imageViewInfo, 0, &vulkan.swapchainImageViews[i]));
		}
	}
	DebugPrint("Created SwapChain\n");

	if (!CreateDepthBuffer())
	{
		return false;
	}
	if (!CreateMSAABuffer())
	{
		return false;
	}
	if (!CreateFrameBuffers())
	{
		return false;
	}
	vulkan.canRender = true;
	return true;
}

static_func void Vulkan::ClearSwapchainImages()
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);
		for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
		{
			if (VulkanIsValidHandle(vulkan.swapchainImageViews[i]))
			{
				vkDestroyImageView(vulkan.device, vulkan.swapchainImageViews[i], 0);
				DebugPrint("Cleared Swapchain Image View\n");
			}
		}
	}
}

static_func void Vulkan::ClearSwapchain()
{
	ClearImage(vulkan.depthBuffer);
	ClearImage(vulkan.msaa);
	ClearFrameBuffers();
	ClearSwapchainImages();
	if (VulkanIsValidHandle(vulkan.swapchain))
		vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);
}

static_func bool Vulkan::LoadShader(char *filepath, VkShaderModule *shaderOut)
{
	debug_read_file_result shaderCode = platformAPI.DEBUGReadFile(filepath);
	Assert(shaderCode.size < SHADER_CODE_BUFFER_SIZE);
	if (shaderCode.content)
	{
		VkShaderModuleCreateInfo shaderInfo = {};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = shaderCode.size;
		shaderInfo.pCode = (u32 *)shaderCode.content;
		VkResult res = vkCreateShaderModule(vulkan.device, &shaderInfo, 0, shaderOut);
		platformAPI.DEBUGFreeFileMemory(shaderCode.content);
		if (res == VK_SUCCESS)
		{
			DebugPrint("Loaded Shader\n");
			return true;
		}
	}
	return false;

}

static_func bool Vulkan::CreatePipeline()
{
	ClearPipeline();

	// NOTE(heyyod): LOAD THE SHADERS
	VkShaderModule triangleVertShader = {};
	VkShaderModule triangleFragShader = {};
	if (!LoadShader(".\\assets\\shaders\\default.frag.spv", &triangleFragShader) ||
		!LoadShader(".\\assets\\shaders\\default.vert.spv", &triangleVertShader))
	{
		DebugPrint("Couldn't load shaders\n");
		Assert("Couldn't load shaders");
		return false;
	}

	// NOTE(heyyod): specify some general info for the memory of
	// the vertex buffer we'll be reading from
	VkVertexInputBindingDescription vertexBindingDesc;
	vertexBindingDesc.binding = 0;
	vertexBindingDesc.stride = sizeof(vertex);
	vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexAttributeDescs[3];
	// NOTE(heyyod): inPosition format in vertex shader
	vertexAttributeDescs[0].binding = 0;
	vertexAttributeDescs[0].location = 0;// NOTE(heyyod): this maches the value in the shader
	vertexAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescs[0].offset = offsetof(vertex, pos);
	// NOTE(heyyod): inColor format in vertex shader
	vertexAttributeDescs[1].binding = 0;
	vertexAttributeDescs[1].location = 1;
	vertexAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescs[1].offset = offsetof(vertex, color);
	// NOTE(heyyod): inTexCoord format in vertex shader
	vertexAttributeDescs[2].binding = 0;
	vertexAttributeDescs[2].location = 2;
	vertexAttributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttributeDescs[2].offset = offsetof(vertex, texCoord);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(vertexAttributeDescs);
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescs;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = triangleVertShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = triangleFragShader;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};
	//-
	// NOTE(heyyod): 128bytes limit (at least on my device)

	VkPushConstantRange pushConstants[1] = {};;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(u32); // index of transform in transforms buffer
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	//-
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	//-
	//NOTE(heyyod): Viewport and scissor are  dynamic and update them with vkCmd commands 
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;
	//-
	VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//rasterizerInfo.depthClampEnable = VK_FALSE;
	//rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//rasterizerInfo.depthBiasEnable = VK_FALSE;
	//rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
	//rasterizerInfo.depthBiasClamp = 0.0f; // Optional
	//rasterizerInfo.depthBiasSlopeFactor = 0.0f; // Optional
	//-
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = vulkan.msaaSamples;
	multisamplingInfo.minSampleShading = 1.0f; // Optional
	//multisamplingInfo.pSampleMask = nullptr; // Optional
	//multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	//multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional
	//-
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	//depthStencil.minDepthBounds = 0.0f; // Optional
	//depthStencil.maxDepthBounds = 1.0f; // Optional
	//depthStencil.stencilTestEnable = VK_FALSE;
	//depthStencil.front = {}; // Optional
	//depthStencil.back = {}; // Optional
	//-

	VkPipelineColorBlendAttachmentState blendAttachment = {};
	blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachment.blendEnable = VK_FALSE;
	//blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	//blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	//blendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	//blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	//blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	//blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo blendingInfo = {};
	blendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendingInfo.logicOpEnable = VK_FALSE;
	blendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	blendingInfo.attachmentCount = 1;
	blendingInfo.pAttachments = &blendAttachment;
	//blendingInfo.blendConstants[0] = 0.0f; // Optional
	//blendingInfo.blendConstants[1] = 0.0f; // Optional
	//blendingInfo.blendConstants[2] = 0.0f; // Optional
	//blendingInfo.blendConstants[3] = 0.0f; // Optional
	//-
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		//VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = ArrayCount(dynamicStates);
	dynamicStateInfo.pDynamicStates = dynamicStates;
	//-
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &vulkan.globalDescSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = ArrayCount(pushConstants);
	pipelineLayoutInfo.pPushConstantRanges = pushConstants;
	AssertSuccess(vkCreatePipelineLayout(vulkan.device, &pipelineLayoutInfo, 0, &vulkan.pipeline[PIPELINE_MESH].layout));

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = ArrayCount(shaderStages);
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &blendingInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo; // Optional
	pipelineInfo.layout = vulkan.pipeline[PIPELINE_MESH].layout;
	pipelineInfo.renderPass = vulkan.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional


	AssertSuccess(vkCreateGraphicsPipelines(vulkan.device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &vulkan.pipeline[PIPELINE_MESH].handle));

	DebugPrint("Created Pipeline\n");
	vkDestroyShaderModule(vulkan.device, triangleVertShader, 0);
	vkDestroyShaderModule(vulkan.device, triangleFragShader, 0);

	return true;
}

static_func void Vulkan::ClearPipeline()
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);
		if (VulkanIsValidHandle(vulkan.pipeline))
		{
			vkDestroyPipeline(vulkan.device, vulkan.pipeline[PIPELINE_MESH].handle, 0);
			DebugPrint("Cleared Pipeline\n");
		}

		if (VulkanIsValidHandle(vulkan.pipeline[PIPELINE_MESH].layout))
		{
			vkDestroyPipelineLayout(vulkan.device, vulkan.pipeline[PIPELINE_MESH].layout, 0);
		}
	}
}

static_func bool Vulkan::CreateBuffer(VkBufferUsageFlags usage, u64 size, VkMemoryPropertyFlags properties, vulkan_buffer &bufferOut, bool mapBuffer)
{
	// TODO(heyyod): make this able to create multiple buffers of the same type and usage if I pass an array
	if (!(VulkanIsValidHandle(bufferOut.handle)))
	{
		bufferOut.size = size;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = usage;
		bufferInfo.size = size;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		AssertSuccess(vkCreateBuffer(vulkan.device, &bufferInfo, 0, &bufferOut.handle));

		VkMemoryRequirements memoryReq = {};
		vkGetBufferMemoryRequirements(vulkan.device, bufferOut.handle, &memoryReq);

		u32 memIndex = 0;
		if (Vulkan::FindMemoryProperties(memoryReq.memoryTypeBits, properties, memIndex))
		{
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memoryReq.size;
			allocInfo.memoryTypeIndex = memIndex;

			AssertSuccess(vkAllocateMemory(vulkan.device, &allocInfo, 0, &bufferOut.memoryHandle));
			AssertSuccess(vkBindBufferMemory(vulkan.device, bufferOut.handle, bufferOut.memoryHandle, 0));

			if (mapBuffer)
				AssertSuccess(vkMapMemory(vulkan.device, bufferOut.memoryHandle, 0, bufferOut.size, 0, &bufferOut.data));
		}
		else
		{
			Assert("Could not allocate vertex buffer memory");
			return false;
		}
	}
	return true;
}

static_func void Vulkan::ClearBuffer(vulkan_buffer buffer)
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		if (VulkanIsValidHandle(buffer.handle))
			vkDestroyBuffer(vulkan.device, buffer.handle, 0);
		if (VulkanIsValidHandle(buffer.memoryHandle))
		{
			vkFreeMemory(vulkan.device, buffer.memoryHandle, 0);
		}
	}
}

static_func void Vulkan::Destroy()
{
	if (VulkanIsValidHandle(vulkan.device))
	{
		vkDeviceWaitIdle(vulkan.device);

		if (VulkanIsValidHandle(vulkan.textureSampler))
			vkDestroySampler(vulkan.device, vulkan.textureSampler, 0);

		ClearImage(vulkan.texture);

		ClearPipeline();

		if (VulkanIsValidHandle(vulkan.globalDescSetLayout))
			vkDestroyDescriptorSetLayout(vulkan.device, vulkan.globalDescSetLayout, 0);

		for (u32 i = 0; i < ArrayCount(vulkan.frameData); i++)
		{
			ClearBuffer(vulkan.frameData[i].cameraBuffer);
			ClearBuffer(vulkan.frameData[i].sceneBuffer);
		}
		ClearBuffer(vulkan.staticTransformsBuffer);

		if (VulkanIsValidHandle(vulkan.globalDescPool))
			vkDestroyDescriptorPool(vulkan.device, vulkan.globalDescPool, 0);

		vkUnmapMemory(vulkan.device, vulkan.stagingBuffer.memoryHandle);
		ClearBuffer(vulkan.stagingBuffer);
		ClearBuffer(vulkan.vertexBuffer);
		ClearBuffer(vulkan.indexBuffer);

		ClearRenderPass();

		ClearSwapchain();


		for (u32 i = 0; i < NUM_RESOURCES; i++)
		{
			if (VulkanIsValidHandle(vulkan.resources[i].imgAvailableSem))
				vkDestroySemaphore(vulkan.device, vulkan.resources[i].imgAvailableSem, 0);
			if (VulkanIsValidHandle(vulkan.resources[i].frameReadySem))
				vkDestroySemaphore(vulkan.device, vulkan.resources[i].frameReadySem, 0);
			if (VulkanIsValidHandle(vulkan.resources[i].fence))
				vkDestroyFence(vulkan.device, vulkan.resources[i].fence, 0);
		}

		if (VulkanIsValidHandle(vulkan.cmdPool))
			vkDestroyCommandPool(vulkan.device, vulkan.cmdPool, 0);

		vkDestroyDevice(vulkan.device, 0);
	}

	if (VulkanIsValidHandle(vulkan.instance))
	{
		if (VulkanIsValidHandle(vulkan.surface))
			vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
		if (VulkanIsValidHandle(vulkan.debugMessenger))
		vkDestroyDebugUtilsMessengerEXT(vulkan.instance, vulkan.debugMessenger, 0);
#endif

		vkDestroyInstance(vulkan.instance, 0);
	}

	if (vulkan.dll)
		FreeLibrary(vulkan.dll);

	DebugPrint("Destroyed Vulkan\n");
	return;
}

static_func bool Vulkan::ChangeGraphicsSettings(graphics_settings settings, CHANGE_GRAPHICS_SETTINGS newSettings)
{
	if (newSettings & CHANGE_MSAA)
	{
		PickMSAA(settings.msaa);
		if (!CreateRenderPass() ||
			!CreateDepthBuffer() ||
			!CreateMSAABuffer() ||
			!CreateFrameBuffers() ||
			!CreatePipeline())
		{
			Assert("Could not change msaa settings");
			return false;
		}
	}
	return true;
}

static_func void Vulkan::CmdChangeImageLayout(VkCommandBuffer cmdBuffer, VkImage imgHandle, image *imageInfo, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	// NOTE(heyyod): ONLY CALL AFRER WE START RECORDING A COMMAND BUFFER
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = imgHandle;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		/* 
        if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
         */
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = imageInfo->mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		Assert("Layout transition not implemented.");
	}

	vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, 0, 0, 0, 1, &barrier);
}

static_func frame_prep_resource * Vulkan::GetNextAvailableResource()
{
	frame_prep_resource *result = &vulkan.resources[vulkan.currentResource];
	vulkan.currentResource = (vulkan.currentResource + 1) % NUM_RESOURCES;

	AssertSuccess(vkWaitForFences(vulkan.device, 1, &result->fence, true, UINT64_MAX));
	AssertSuccess(vkResetFences(vulkan.device, 1, &result->fence));

	return result;
}

static_func bool Vulkan::PushStaged(staged_resources &staged)
{
	frame_prep_resource *res = GetNextAvailableResource();

	VkMappedMemoryRange flushRange = {};
	flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	flushRange.memory = vulkan.stagingBuffer.memoryHandle;
	flushRange.size = vulkan.stagingBuffer.size;
	vkFlushMappedMemoryRanges(vulkan.device, 1, &flushRange);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	AssertSuccess(vkBeginCommandBuffer(res->cmdBuffer, &cmdBufferBeginInfo));

	bool pushedMesh = false;
	bool pushedImage = false;

	u64 vertexBufferStartOffset = vulkan.vertexBuffer.writeOffset;
	u64 indexBufferStartOffset = vulkan.indexBuffer.writeOffset;


	for (u32 resourceId = 0; resourceId < staged.count; resourceId++)
	{

		switch (staged.types[resourceId])
		{
			//-
			case RESOURCE_MESH:
			{
				pushedMesh = true;
				mesh *m = (mesh *)staged.resources[resourceId];

				VkBufferCopy bufferCopyInfo = {};
				// NOTE(heyyod): Set where to read and where to copy the indices
				bufferCopyInfo.srcOffset = staged.offsets[resourceId];
				bufferCopyInfo.dstOffset = vulkan.indexBuffer.writeOffset;
				bufferCopyInfo.size = MESH_PTR_INDICES_SIZE(m);
				vulkan.indexBuffer.writeOffset += bufferCopyInfo.size; //set next write offset in index buffer
				vkCmdCopyBuffer(res->cmdBuffer, vulkan.stagingBuffer.handle, vulkan.indexBuffer.handle, 1, &bufferCopyInfo);

				// NOTE(heyyod): Set where to read and where to copy the vertices
				bufferCopyInfo.srcOffset = bufferCopyInfo.srcOffset + bufferCopyInfo.size;
				bufferCopyInfo.dstOffset = vulkan.vertexBuffer.writeOffset;
				bufferCopyInfo.size = MESH_PTR_VERTICES_SIZE(m);
				vulkan.vertexBuffer.writeOffset += bufferCopyInfo.size; //set next write offset in vertex buffer
				vkCmdCopyBuffer(res->cmdBuffer, vulkan.stagingBuffer.handle, vulkan.vertexBuffer.handle, 1, &bufferCopyInfo);

				vulkan.loadedMesh[vulkan.loadedMeshCount].nIndices = m->nIndices;
				vulkan.loadedMesh[vulkan.loadedMeshCount].nVertices = m->nVertices;
				vulkan.loadedMesh[vulkan.loadedMeshCount].nInstances = staged.nInstances[resourceId];
				vulkan.loadedMesh[vulkan.loadedMeshCount].pipelineID = PIPELINE_MESH;

				//u32 objectTransformsSize = staged.nInstances[resourceId] * sizeof(object_transform);
				//void *dst = vulkan.staticTransformsBuffer.data;
				//AdvancePointer(dst, vulkan.staticTransformsBuffer.writeOffset);

				//vulkan.loadedMesh[vulkan.loadedMeshCount].transforms = dst - vulkan.staticTransformsBuffer.data;

				//memcpy(dst, staged.transforms, objectTransformsSize);

				vulkan.loadedMeshCount++;
			} break;
			//-
			case RESOURCE_TEXTURE:
			{
				pushedImage = true;
				image *img = (image *)staged.resources[resourceId];
				if (!CreateImage(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { img->width, img->height, 1 }, img->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | // to create the mipmaps
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | // to copy buffer into image
					VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					VK_IMAGE_ASPECT_COLOR_BIT,
					vulkan.texture))
				{
					return false;
				}

				// NOTE(heyyod): We need to write into the image from the stage buffer.
				// So we change the layout.
				{
					CmdChangeImageLayout(res->cmdBuffer, vulkan.texture.handle, img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

					VkBufferImageCopy bufferToImageCopy = {};
					bufferToImageCopy.bufferOffset = staged.offsets[resourceId];
					bufferToImageCopy.bufferRowLength = 0;
					bufferToImageCopy.bufferImageHeight = 0;
					bufferToImageCopy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
					bufferToImageCopy.imageOffset = { 0, 0, 0 };
					bufferToImageCopy.imageExtent = { img->width, img->height, 1 };
					vkCmdCopyBufferToImage(res->cmdBuffer, vulkan.stagingBuffer.handle, vulkan.texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferToImageCopy);
				}

				// NOTE(heyyod): Generate mipmaps
				// NOTE(heyyod): Check if image format supports linear blitting
				VkFormatProperties formatProperties;
				vkGetPhysicalDeviceFormatProperties(vulkan.gpu, VK_FORMAT_R8G8B8A8_UNORM, &formatProperties);
				if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
				{
					i32 mipWidth = (i32)img->width;
					i32 mipHeight = (i32)img->height;

					VkImageMemoryBarrier sourceMipBarrier{};
					sourceMipBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					sourceMipBarrier.image = vulkan.texture.handle;
					sourceMipBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					sourceMipBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					sourceMipBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					sourceMipBarrier.subresourceRange.baseArrayLayer = 0;
					sourceMipBarrier.subresourceRange.layerCount = 1;
					sourceMipBarrier.subresourceRange.levelCount = 1;

					VkImageBlit mipMapBlit = {};
					mipMapBlit.srcOffsets[0] = { 0, 0, 0 };
					mipMapBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					mipMapBlit.srcSubresource.baseArrayLayer = 0;
					mipMapBlit.srcSubresource.layerCount = 1;
					mipMapBlit.dstOffsets[0] = { 0, 0, 0 };
					mipMapBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					mipMapBlit.dstSubresource.baseArrayLayer = 0;
					mipMapBlit.dstSubresource.layerCount = 1;
					for (u32 i = 1; i < img->mipLevels; i++)
					{
						// NOTE(heyyod): Change previous mip from dst to src since we'll read from it
						sourceMipBarrier.subresourceRange.baseMipLevel = i - 1;
						sourceMipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
						sourceMipBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						sourceMipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
						sourceMipBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 0, 0, 1, &sourceMipBarrier);

						// NOTE(heyyod): Specify the reagion we'll read from the previous mip
						mipMapBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
						mipMapBlit.srcSubresource.mipLevel = i - 1;
						mipMapBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
						mipMapBlit.dstSubresource.mipLevel = i;

						// NOTE(heyyod): Do the blit
						vkCmdBlitImage(res->cmdBuffer,
							vulkan.texture.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkan.texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							1, &mipMapBlit, VK_FILTER_LINEAR);

						// NOTE(heyyod): Change layout of previous mip to shader bit since we're done with it.
						sourceMipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						sourceMipBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						sourceMipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						sourceMipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, 0, 0, 0, 1, &sourceMipBarrier);

						if (mipWidth > 1)
							mipWidth /= 2;
						if (mipHeight > 1)
							mipHeight /= 2;
					}

					// NOTE(heyyod): change layout of last mip we created
					sourceMipBarrier.subresourceRange.baseMipLevel = img->mipLevels - 1;
					sourceMipBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					sourceMipBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					sourceMipBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					sourceMipBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, 0, 0, 0, 1, &sourceMipBarrier);
				}
				else
				{
					/* 
                    There are two alternatives in this case. You could implement a static_func that searches common texture image formats for one that does support linear blitting, or you could implement the mipmap generation in software with a library like stb_image_resize. Each mip level can then be loaded into the image in the same way that you loaded the original image.
                    
                    It should be noted that it is uncommon in practice to generate the mipmap levels at runtime anyway. Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed. Implementing resizing in software and loading multiple levels from a file is left as an exercise to the reader.
                     */
				}

				// NOTE(heyyod): Since it will be used in shaders we again need to change the layout
				//CmdChangeImageLayout(res->cmdBuffer, vulkan.texture.handle, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				// NOTE(heyyod): Update the descriptor set
				{
					VkDescriptorImageInfo descImageInfo = {};
					descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					descImageInfo.imageView = vulkan.texture.view;
					descImageInfo.sampler = vulkan.textureSampler;

					VkWriteDescriptorSet writeDesc = {};
					writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDesc.dstBinding = 2;
					writeDesc.dstArrayElement = 0;
					writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writeDesc.descriptorCount = 1;
					writeDesc.pImageInfo = &descImageInfo;

					for (u8 i = 0; i < ArrayCount(vulkan.frameData); i++)
					{
						writeDesc.dstSet = vulkan.frameData[i].globalDescriptor;
						vkUpdateDescriptorSets(vulkan.device, 1, &writeDesc, 0, 0);
					}
				}
			} break;

			default:
			{
				DebugPrint("ERROR: Trying to push unknown resource ata type.\n");
				//return false;
			}
		}
	}

	if (pushedMesh)
	{
		VkBufferMemoryBarrier bufferBarriers[2] = {};

		bufferBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarriers[0].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		bufferBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		bufferBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[0].buffer = vulkan.vertexBuffer.handle;
		bufferBarriers[0].offset = vertexBufferStartOffset;
		bufferBarriers[0].size = vulkan.vertexBuffer.writeOffset - vertexBufferStartOffset;

		bufferBarriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarriers[1].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		bufferBarriers[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
		bufferBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[1].buffer = vulkan.indexBuffer.handle;
		bufferBarriers[1].offset = indexBufferStartOffset;
		bufferBarriers[1].size = vulkan.indexBuffer.writeOffset - indexBufferStartOffset;

		vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, 0, ArrayCount(bufferBarriers), bufferBarriers, 0, 0);
	}

	AssertSuccess(vkEndCommandBuffer(res->cmdBuffer));
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &res->cmdBuffer;
	AssertSuccess(vkQueueSubmit(vulkan.graphicsQueue, 1, &submitInfo, res->fence));

	return true;
}


static_func bool Vulkan::Draw(update_data *data)
{
	frame_prep_resource *res = GetNextAvailableResource();

	// NOTE: Get the next image that we'll use to create the frame and draw it
	// Signal the semaphore when it's available
	local_var u32 nextImage = 0;
	local_var VkResult result = {};
	{
		result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX, res->imgAvailableSem, 0, &nextImage);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!CreateSwapchain())
				return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			Assert("Could not aquire next image");
			return false;
		}
	}

	// NOTE(heyyod): Prepare the frame using the selected resources
	{
		local_var VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// NOTE(heyyod): THE ORDER OF THESE VALUES MUST BE IDENTICAL
		// TO THE ORDER WE SPECIFIED THE RENDERPASS ATTACHMENTS
		local_var VkClearValue clearValues[2] = {}; // NOTE(heyyod): this is a union
		clearValues[0].color = VulkanClearColor(data->clearColor[0], data->clearColor[1], data->clearColor[2], 0.0f);
		clearValues[1].depthStencil = { 1.0f, 0 };

		local_var VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = vulkan.renderPass;
		renderpassInfo.renderArea.extent = vulkan.windowExtent;
		renderpassInfo.clearValueCount = ArrayCount(clearValues);
		renderpassInfo.pClearValues = clearValues;
		renderpassInfo.framebuffer = vulkan.framebuffers[nextImage];

		local_var VkViewport viewport = {};
		viewport.width = (f32) vulkan.windowExtent.width;
		viewport.height = (f32) vulkan.windowExtent.height;
		viewport.maxDepth = 1.0f;

		local_var VkRect2D scissor = {};
		scissor.extent = vulkan.windowExtent;

		local_var VkDeviceSize bufferOffset = 0;

		AssertSuccess(vkBeginCommandBuffer(res->cmdBuffer, &commandBufferBeginInfo));
		vkCmdBeginRenderPass(res->cmdBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(res->cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(res->cmdBuffer, 0, 1, &scissor);

		vkCmdBindVertexBuffers(res->cmdBuffer, 0, 1, &vulkan.vertexBuffer.handle, &bufferOffset);
		vkCmdBindIndexBuffer(res->cmdBuffer, vulkan.indexBuffer.handle, 0, VULKAN_INDEX_TYPE);
		u32 firstIndex = 0;
		u32 indexOffset = 0;
		i32 currentPipelineId = -1;
		u32 transformId = 0;
		for (u32 meshId = 0; meshId < vulkan.loadedMeshCount; meshId++)
		{
			if (currentPipelineId != (i32)vulkan.loadedMesh[meshId].pipelineID)
			{
				currentPipelineId = vulkan.loadedMesh[meshId].pipelineID;
				vkCmdBindPipeline(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.pipeline[currentPipelineId].handle);
				vkCmdBindDescriptorSets(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					vulkan.pipeline[currentPipelineId].layout, 0, 1,
					&vulkan.frameData[nextImage].globalDescriptor, 0, 0);
			}
			for (u32 meshInstance = 0; meshInstance < vulkan.loadedMesh[meshId].nInstances; meshInstance++)
			{
				vkCmdPushConstants(res->cmdBuffer, vulkan.pipeline[currentPipelineId].layout, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(u32), &transformId);
				vkCmdDrawIndexed(res->cmdBuffer, vulkan.loadedMesh[meshId].nIndices, 1,
					firstIndex, indexOffset, 0);

				// NOTE(heyyod): THIS ASSUMES THE TRANFORMS ARE ALWAYS LINEARLY SAVED :(
				transformId++;
			}
			firstIndex += vulkan.loadedMesh[meshId].nIndices;
			indexOffset += vulkan.loadedMesh[meshId].nVertices;
		}

		vkCmdEndRenderPass(res->cmdBuffer);
		AssertSuccess(vkEndCommandBuffer(res->cmdBuffer));
	}

	// NOTE: Wait on imageAvailableSem and submit the command buffer and signal renderFinishedSem
	{
		local_var VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		local_var VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitDestStageMask;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &res->imgAvailableSem;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &res->frameReadySem;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &res->cmdBuffer;
		AssertSuccess(vkQueueSubmit(vulkan.graphicsQueue, 1, &submitInfo, res->fence));

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!CreateSwapchain())
			{
				Assert("Could not recreate after queue sumbit.\n");
				return false;
			}
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			Assert("Couldn't submit draw.\n");
			return false;
		}
	}

	// NOTE: Submit image to present when signaled by renderFinishedSem
	{
		local_var VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &res->frameReadySem;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vulkan.swapchain;
		presentInfo.pImageIndices = &nextImage;
		result = vkQueuePresentKHR(vulkan.presentQueue, &presentInfo);


		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			if (!CreateSwapchain())
			{
				Assert("Could not recreate after queue present.\n");
				return false;
			}
		}
		else if (result != VK_SUCCESS)
		{
			Assert("Couldn't present image.\n");
			return false;
		}

	}

	// NOTE(heyyod): Update data for the engine
	{
		u32 nextUniformBuffer = (nextImage + 1) % NUM_DESCRIPTORS;
		data->newCameraBuffer = vulkan.frameData[nextUniformBuffer].cameraBuffer.data;
		data->newSceneBuffer = vulkan.frameData[nextUniformBuffer].sceneBuffer.data;
	}

	return true;
}

#if 0
static_func bool Vulkan::CreateCommandBuffers()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkan.presentQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	AssertSuccess(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;
	for (u32 i = 0; i < NUM_RESOURCES; i++)
	{
		AssertSuccess(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, &vulkan.resources[i].cmdBuffer));
	}
	DebugPrint("Created Command Pool and Command Buffers\n");

	return true;
}


static_func bool Vulkan::ResetCommandBuffers()
{
	if (VulkanIsValidHandle(vulkan.device) && VulkanIsValidHandle(vulkan.cmdPool))
	{
		vkDeviceWaitIdle(vulkan.device);
		vkResetCommandPool(vulkan.device, vulkan.cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		DebugPrint("Reseted Command Buffers\n");
		return true;
	}
	return false;
}

static_func void Vulkan::GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc)
{
	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(v);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

static_func void Vulkan::GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs)
{
	//Assert(ArrayCount(attributeDescs) == 2); // for pos and color
	attributeDescs[0].binding = 0;
	attributeDescs[0].location = 0;
	attributeDescs[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescs[0].offset = offsetof(vertex2, pos);

	attributeDescs[1].binding = 0;
	attributeDescs[1].location = 1;
	attributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescs[1].offset = offsetof(vertex2, color);
}

#endif
