#include "vulkan_platform.h"

#if VULKAN_VALIDATION_LAYERS_ON
static_func VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
	type = "Potentially non-optimal use of vulkanContext->";

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
	Assert(0);
	return VK_FALSE;
}
#endif

static_func void VulkanPickMSAA(MSAA_OPTIONS msaa)
{
	VkSampleCountFlags counts = vulkanContext->gpuProperties.limits.framebufferColorSampleCounts & vulkanContext->gpuProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT && msaa >= MSAA_64)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT && msaa >= MSAA_32)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT && msaa >= MSAA_16)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT && msaa >= MSAA_8)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT && msaa >= MSAA_4)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT && msaa >= MSAA_2)
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else
		vulkanContext->msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

inline static_func bool VulkanLoadCode()
{
	// TODO: make this cross-platform
	if (!vulkanContext->dll)
		vulkanContext->dll = LoadLibraryA("vulkan-1.dll");
	if (!vulkanContext->dll)
	{
		DebugPrintFunctionResult(false);
		return false;
	}

	vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkanContext->dll, "vkGetInstanceProcAddr");

	DebugPrintFunctionResult(true);
	return true;
}

inline static_func bool VulkanCreateInstance(renderer_platform *renderer, const char *appName)
{
	//------------------------------------------------------------------------
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = appName;
	appInfo.engineVersion = VK_API_VERSION_1_0;
	appInfo.apiVersion = VK_API_VERSION_1_2;

	//------------------------------------------------------------------------
#if VULKAN_VALIDATION_LAYERS_ON
	char *instanceLayers[] = {
		"VK_LAYER_KHRONOS_validation" };

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
	debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerInfo.messageSeverity =
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
	;
	debugMessengerInfo.messageType =
	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerInfo.pfnUserCallback = VulkanDebugCallback;
#else
	// char *instanceLayers[] = {};
	char **instanceLayers = 0;
#endif

	if (instanceLayers)
	{
		u32 layerCount;
		VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, 0));

		VkLayerProperties availableLayers[32];
		Assert(layerCount <= ArrayCount(availableLayers));
		VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

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

	//------------------------------------------------------------------------
	char *extensions[] = {
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

	u32 availableExtensionsCount;
	VkExtensionProperties availableInstanceExtensions[255] = {};
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(0, &availableExtensionsCount, 0));
	Assert(availableExtensionsCount <= ArrayCount(availableInstanceExtensions));
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(0, &availableExtensionsCount, availableInstanceExtensions));
	for (char *desiredInstanceExtension : extensions)
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

	//------------------------------------------------------------------------
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = ArrayCount(extensions);
	instanceInfo.ppEnabledExtensionNames = extensions;
	if (instanceLayers)
	{
		instanceInfo.enabledLayerCount = ArrayCount(instanceLayers);
		instanceInfo.ppEnabledLayerNames = instanceLayers;
	}
#if VULKAN_VALIDATION_LAYERS_ON
	instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif

	VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, 0, &vulkanContext->instance));

	DebugPrintFunctionResult(true);

	if (!VulkanLoadInstanceFunctions())
	{
		Assert("ERROR: Instance Functions not loaded\n");
		return false;
	}

#if VULKAN_VALIDATION_LAYERS_ON
	VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(vulkanContext->instance, &debugMessengerInfo, 0, &vulkanContext->debugMessenger));
#endif

	return true;
}

inline static_func bool VulkanCreateSurface(renderer_platform *renderer)
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	renderer->FillSurfaceWindowContext((void *)&surfaceInfo);
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(vulkanContext->instance, &surfaceInfo, 0, &vulkanContext->surface));

#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.connection = 0; // we'll have these if we implement a linux window
	surfaceInfo.window = 0;		// we'll have these if we implement a linux window
	VK_CHECK_RESULT(vkCreateXcbSurfaceKHR(vulkanContext->instance, &surfaceInfo, 0, &vulkanContext->surface));

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.dpy = 0;	// we'll have these if we implement a mac(?) window
	surfaceInfo.window = 0; // we'll have these if we implement a mac(?) window
	VK_CHECK_RESULT(vkCreateXlibSurfaceKHR(vulkanContext->instance, &surfaceInfo, 0, &vulkanContext->surface));
#endif

	vulkanContext->surfaceFormat.colorSpace = SURFACE_FORMAT_COLOR_SPACE;
	vulkanContext->surfaceFormat.format = SURFACE_FORMAT_FORMAT;

	u32 formatCount = 0;
	VkSurfaceFormatKHR availableFormats[16] = {};
	bool desiredSurfaceFormatSupported = false;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext->gpu, vulkanContext->surface, &formatCount, 0));
	Assert(formatCount <= ArrayCount(availableFormats));

	for (u32 i = 0; i < formatCount; ++i)
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanContext->gpu, vulkanContext->surface, &formatCount, &availableFormats[i]));
	for (u32 i = 0; i < formatCount; ++i)
	{
		if (vulkanContext->surfaceFormat.format == availableFormats[i].format &&
			vulkanContext->surfaceFormat.colorSpace == availableFormats[i].colorSpace)
			desiredSurfaceFormatSupported = true;
	}
	Assert(desiredSurfaceFormatSupported);

	if (!desiredSurfaceFormatSupported)
	{
		DebugPrintFunctionResult(false);
		return false;
	}

	DebugPrintFunctionResult(true);
	return true;
}

inline static_func bool VulkanPickCommandQueues()
{
	// TODO: Pick transfer queue from seperate family from that of graphics queue.
	bool result = true;

	vulkanContext->graphicsQueueFamilyIndex = UINT32_MAX;
	vulkanContext->presentQueueFamilyIndex = UINT32_MAX;
	vulkanContext->transferQueueFamilyIndex = UINT32_MAX;
	{
		u32 queueFamilyCount;
		VkQueueFamilyProperties availableQueueFamilies[16] = {};
		VkBool32 supportsPresent[16] = {};
		vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext->gpu, &queueFamilyCount, 0);
		Assert(queueFamilyCount <= ArrayCount(availableQueueFamilies));
		vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext->gpu, &queueFamilyCount, availableQueueFamilies);

		for (u32 i = 0; i < queueFamilyCount; i++)
			VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(vulkanContext->gpu, i, vulkanContext->surface, &supportsPresent[i]));

		for (u32 i = 0; i < queueFamilyCount; ++i)
		{
			// NOTE(heyyod): Find a graphics queue
			if ((availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (vulkanContext->graphicsQueueFamilyIndex == UINT32_MAX)
					vulkanContext->graphicsQueueFamilyIndex = i;
				if (supportsPresent[i] == VK_TRUE)
				{
					vulkanContext->graphicsQueueFamilyIndex = i;
					vulkanContext->presentQueueFamilyIndex = i;
					break;
				}
			}
		}
		for (u32 i = 0; i < queueFamilyCount; ++i)
		{
			// NOTE(heyyod): Find a transfer queue
			if ((availableQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				if (vulkanContext->transferQueueFamilyIndex == UINT32_MAX)
					vulkanContext->transferQueueFamilyIndex = i;
			}
		}
		if (vulkanContext->presentQueueFamilyIndex == UINT32_MAX) // didn't find a queue that supports both graphics and present
		{
			for (u32 i = 0; i < queueFamilyCount; ++i)
				if (supportsPresent[i] == VK_TRUE)
				{
					vulkanContext->presentQueueFamilyIndex = i;
					break;
				}
		}
		if (vulkanContext->graphicsQueueFamilyIndex == UINT32_MAX)
		{
			Assert("ERROR: No graphics queue found.\n");
			result = false;
		}
		if (result && vulkanContext->presentQueueFamilyIndex == UINT32_MAX)
		{
			Assert("ERROR: No present queue found.\n");
			result = false;
		}
	}

	// TODO: make this work for seperate queues if needed
	if (result && vulkanContext->graphicsQueueFamilyIndex != vulkanContext->presentQueueFamilyIndex)
	{
		Assert("vulkanContext->graphicsQueueFamilyIndex != vulkanContext->presentQueueFamilyIndex\n");
		result = false;
	}

	// TODO: Need to do some stuff if they are different like:
	// VkDeviceQueueCreateInfo queueInfo[2] = {};
	// float queuePriority = 1.0; // must be array of size queueCount
	//
	// queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	// queueInfo[0].queueFamilyIndex = graphicsQueueFamilyIndex;
	// queueInfo[0].queueCount = 1;
	// queueInfo[0].pQueuePriorities = &queuePriority;
	//
	// queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	// queueInfo[1].queueFamilyIndex = presentQueueFamilyIndex;
	// queueInfo[1].queueCount = 1;
	// queueInfo[1].pQueuePriorities = &queuePriority;

	DebugPrintFunctionResult(result);
	return result;
}

inline static_func bool VulkanCreateDevice(renderer_platform *renderer)
{
	bool result;

	// PICK PHYSICAL DEVICE
	{
		u32 gpuCount = 0;
		VkPhysicalDevice gpuBuffer[16] = {};
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vulkanContext->instance, &gpuCount, 0));
		Assert(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vulkanContext->instance, &gpuCount, gpuBuffer));
		vulkanContext->gpu = gpuBuffer[0];

		// TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
		// For now it's ok since I only have 1 gpu.

		vkGetPhysicalDeviceProperties(vulkanContext->gpu, &vulkanContext->gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(vulkanContext->gpu, &vulkanContext->memoryProperties);

		DebugPrint("GPU: " << vulkanContext->gpuProperties.deviceName << "\n");
	}

	result = VulkanCreateSurface(renderer);

	if (result)
		result = VulkanPickCommandQueues();

	// NOTE: Create a device
	VkDeviceQueueCreateInfo queueInfo = {};

	float queuePriority = 1.0; // must be array of size queueCount
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = vulkanContext->graphicsQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &queuePriority;

	char *desiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	u32 deviceExtensionsCount;
	VkExtensionProperties availableDeviceExtensions[255] = {};
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(vulkanContext->gpu, 0, &deviceExtensionsCount, 0));
	Assert(deviceExtensionsCount <= ArrayCount(availableDeviceExtensions));
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(vulkanContext->gpu, 0, &deviceExtensionsCount, availableDeviceExtensions));
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
	desiredFeatures.fillModeNonSolid = VK_TRUE;
	desiredFeatures.wideLines = VK_TRUE;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = ArrayCount(desiredDeviceExtensions);
	deviceInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
	deviceInfo.pEnabledFeatures = &desiredFeatures;
	VK_CHECK_RESULT(vkCreateDevice(vulkanContext->gpu, &deviceInfo, 0, &vulkanContext->device));

	if (!VulkanLoadDeviceFunctions())
	{
		return false;
	}

	vkGetDeviceQueue(vulkanContext->device, vulkanContext->graphicsQueueFamilyIndex, 0, &vulkanContext->graphicsQueue);
	vkGetDeviceQueue(vulkanContext->device, vulkanContext->presentQueueFamilyIndex, 0, &vulkanContext->presentQueue);

	DebugPrintFunctionResult(true);
	return true;
}

inline static_func bool VulkanCreateCommandResources()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkanContext->presentQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkanContext->device, &cmdPoolInfo, 0, &vulkanContext->cmdPool));

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = vulkanContext->cmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (u32 i = 0; i < NUM_RESOURCES; i++)
	{
		VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkanContext->device, &cmdBufferAllocInfo, &vulkanContext->resources[i].cmdBuffer));
		VK_CHECK_RESULT(vkCreateSemaphore(vulkanContext->device, &semaphoreInfo, 0, &vulkanContext->resources[i].imgAvailableSem));
		VK_CHECK_RESULT(vkCreateSemaphore(vulkanContext->device, &semaphoreInfo, 0, &vulkanContext->resources[i].frameReadySem));
		VK_CHECK_RESULT(vkCreateFence(vulkanContext->device, &fenceInfo, 0, &vulkanContext->resources[i].fence));
	}

	DebugPrintFunctionResult(true);
	return true;
}

static_func void VulkanClearRenderPass()
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		if (VK_CHECK_HANDLE(vulkanContext->renderPass))
			vkDestroyRenderPass(vulkanContext->device, vulkanContext->renderPass, 0);
	}
}

static_func bool VulkanCreateRenderPass()
{
	VulkanClearRenderPass();

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.flags = 0;
	colorAttachment.format = vulkanContext->surfaceFormat.format;
	colorAttachment.samples = vulkanContext->msaaSamples;
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
	depthAttachment.samples = vulkanContext->msaaSamples;
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
	// referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
	subpass.colorAttachmentCount = 1; // ArrayCount(colorAttachementRef)
	subpass.pColorAttachments = &colorAttachementRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// subpass.inputAttachmentCount = 0;
	// subpass.pInputAttachments = 0;
	// subpass.preserveAttachmentCount = 0;
	// subpass.pPreserveAttachments = 0;

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

	if (vulkanContext->msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
		renderPassInfo.attachmentCount = ArrayCount(attachments);
		renderPassInfo.pAttachments = attachments;
		VK_CHECK_RESULT(vkCreateRenderPass(vulkanContext->device, &renderPassInfo, 0, &vulkanContext->renderPass));
	}
	else
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// NOTE(heyyod): attachment to resolve the multisamplerd image into a presentable image
		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = vulkanContext->surfaceFormat.format;
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
		VK_CHECK_RESULT(vkCreateRenderPass(vulkanContext->device, &renderPassInfo, 0, &vulkanContext->renderPass));
	}

	DebugPrintFunctionResult(true);
	return true;
}

static_func bool VulkanLoadShader(char *filepath, VkShaderModule *shaderOut)
{
	debug_read_file_result shaderCode = platformAPI->ReadFile(filepath);
	Assert(shaderCode.size < SHADER_CODE_BUFFER_SIZE);
	if (shaderCode.content)
	{
		VkShaderModuleCreateInfo shaderInfo = {};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = shaderCode.size;
		shaderInfo.pCode = (u32 *)shaderCode.content;
		VkResult res = vkCreateShaderModule(vulkanContext->device, &shaderInfo, 0, shaderOut);
		platformAPI->FreeFileMemory(shaderCode.content);
		if (res == VK_SUCCESS)
		{
			DebugPrint("LOADED SHADER: " << filepath << "\n");
			return true;
		}
	}
	return false;
}

static_func void VulkanClearPipeline(VULKAN_PIPELINE_ID pipelineID)
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		if (VK_CHECK_HANDLE(vulkanContext->pipeline))
		{
			vkDestroyPipeline(vulkanContext->device, vulkanContext->pipeline[pipelineID].handle, 0);
		}

		if (VK_CHECK_HANDLE(vulkanContext->pipeline[pipelineID].layout))
		{
			vkDestroyPipelineLayout(vulkanContext->device, vulkanContext->pipeline[pipelineID].layout, 0);
		}
	}
}

static_func bool VulkanCreatePipeline(VULKAN_PIPELINE_ID pipelineID)
{
	VulkanClearPipeline(pipelineID);

	switch (pipelineID)
	{
		case PIPELINE_GRID:
		{
			// NOTE(heyyod): 128bytes limit (at least on my device)
			VkPushConstantRange pushConstants[1] = {};
			;
			pushConstants[0].offset = 0;
			pushConstants[0].size = sizeof(vec3); // index of transform in transforms buffer
			pushConstants[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &vulkanContext->globalDescriptorSetLayout;
			pipelineLayoutInfo.pushConstantRangeCount = ArrayCount(pushConstants);
			pipelineLayoutInfo.pPushConstantRanges = pushConstants;
			VK_CHECK_RESULT(vkCreatePipelineLayout(vulkanContext->device, &pipelineLayoutInfo, 0, &vulkanContext->pipeline[pipelineID].layout));

			//------------------------------------------------------------------------

			VkShaderModule vs = {};
			VkShaderModule fs = {};

			if (!VulkanLoadShader(".\\assets\\shaders\\grid.vert.spv", &vs) ||
				!VulkanLoadShader(".\\assets\\shaders\\grid.frag.spv", &fs))
			{
				DebugPrint("Couldn't load shaders\n");
				Assert("Couldn't load shaders");
				return false;
			}

			// NOTE(heyyod): specify some general info for the memory of
			// the vertex buffer we'll be reading from
			VkVertexInputBindingDescription vertexBindingDesc;
			vertexBindingDesc.binding = 0;
			vertexBindingDesc.stride = sizeof(vec3);
			vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription vertexAttributeDescs = {};
			// NOTE(heyyod): inPosition format in vertex shader
			vertexAttributeDescs.binding = 0;
			vertexAttributeDescs.location = 0;
			vertexAttributeDescs.format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttributeDescs.offset = 0;

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
			vertexInputInfo.vertexAttributeDescriptionCount = 1;
			vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescs;

			VkPipelineShaderStageCreateInfo vsStage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vsStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vsStage.module = vs;
			vsStage.pName = "main";

			VkPipelineShaderStageCreateInfo fsStage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fsStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fsStage.module = fs;
			fsStage.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = {
				vsStage,
				fsStage
			};
			//------------------------------------------------------------------------
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

			//------------------------------------------------------------------------
			// NOTE(heyyod): Viewport and scissor are  dynamic and update them with vkCmd commands
			VkPipelineViewportStateCreateInfo viewportInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportInfo.viewportCount = 1;
			viewportInfo.scissorCount = 1;

			//------------------------------------------------------------------------
			VkPipelineRasterizationStateCreateInfo rasterizerInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizerInfo.polygonMode = VK_POLYGON_MODE_LINE;
			rasterizerInfo.lineWidth = 0.8f;
			rasterizerInfo.cullMode = VK_CULL_MODE_NONE;
			rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			// rasterizerInfo.depthClampEnable = VK_FALSE;
			// rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
			// rasterizerInfo.depthBiasEnable = VK_FALSE;
			// rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
			// rasterizerInfo.depthBiasClamp = 0.0f; // Optional
			// rasterizerInfo.depthBiasSlopeFactor = 0.0f; // Optional

			//------------------------------------------------------------------------
			VkPipelineMultisampleStateCreateInfo multisamplingInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			// multisamplingInfo.sampleShadingEnable = VK_FALSE;
			multisamplingInfo.rasterizationSamples = vulkanContext->msaaSamples;
			multisamplingInfo.minSampleShading = 1.0f; // Optional
			// multisamplingInfo.pSampleMask = nullptr; // Optional
			// multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
			// multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

			//------------------------------------------------------------------------
			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			// depthStencil.minDepthBounds = 0.0f; // Optional
			// depthStencil.maxDepthBounds = 1.0f; // Optional
			// depthStencil.stencilTestEnable = VK_FALSE;
			// depthStencil.front = {}; // Optional
			// depthStencil.back = {}; // Optional

			//------------------------------------------------------------------------
			VkPipelineColorBlendAttachmentState blendAttachment = {};
			blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachment.blendEnable = VK_TRUE;
			blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo blendingInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			blendingInfo.logicOpEnable = VK_FALSE;
			blendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
			blendingInfo.attachmentCount = 1;
			blendingInfo.pAttachments = &blendAttachment;
			// blendingInfo.blendConstants[0] = 0.0f; // Optional
			// blendingInfo.blendConstants[1] = 0.0f; // Optional
			// blendingInfo.blendConstants[2] = 0.0f; // Optional
			// blendingInfo.blendConstants[3] = 0.0f; // Optional

			//------------------------------------------------------------------------
			VkDynamicState dynamicStates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
				// VK_DYNAMIC_STATE_LINE_WIDTH
			};
			VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
			dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateInfo.dynamicStateCount = ArrayCount(dynamicStates);
			dynamicStateInfo.pDynamicStates = dynamicStates;

			//------------------------------------------------------------------------
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
			pipelineInfo.layout = vulkanContext->pipeline[pipelineID].layout;
			pipelineInfo.renderPass = vulkanContext->renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
			pipelineInfo.basePipelineIndex = -1;			  // Optional

			VK_CHECK_RESULT(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &vulkanContext->pipeline[pipelineID].handle));

			vkDestroyShaderModule(vulkanContext->device, vs, 0);
			vkDestroyShaderModule(vulkanContext->device, fs, 0);

			DebugPrintFunctionResult(true);
			return true;
		}
		break;
		case PIPELINE_WIREFRAME:
		case PIPELINE_MESH:
		{
			// NOTE(heyyod): 128bytes limit (at least on my device)
			VkPushConstantRange pushConstants[1] = {};
			pushConstants[0].offset = 0;
			pushConstants[0].size = sizeof(u32); // index of transform in transforms buffer
			pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &vulkanContext->globalDescriptorSetLayout;
			pipelineLayoutInfo.pushConstantRangeCount = ArrayCount(pushConstants);
			pipelineLayoutInfo.pPushConstantRanges = pushConstants;
			VK_CHECK_RESULT(vkCreatePipelineLayout(vulkanContext->device, &pipelineLayoutInfo, 0, &vulkanContext->pipeline[pipelineID].layout));


			VkShaderModule vs = {};
			VkShaderModule fs = {};

			if (pipelineID == PIPELINE_MESH)
			{
				if (!VulkanLoadShader(".\\assets\\shaders\\default.vert.spv", &vs) ||
					!VulkanLoadShader(".\\assets\\shaders\\default.frag.spv", &fs))
				{
					DebugPrint("Couldn't load shaders\n");
					Assert("Couldn't load shaders");
					return false;
				}
			}
			else // WIREFRAME
			{
				if (!VulkanLoadShader(".\\assets\\shaders\\wireframe.vert.spv", &vs) ||
					!VulkanLoadShader(".\\assets\\shaders\\wireframe.frag.spv", &fs))
				{
					DebugPrint("Couldn't load shaders\n");
					Assert("Couldn't load shaders");
					return false;
				}
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
			vertexAttributeDescs[0].location = 0; // NOTE(heyyod): this maches the value in the shader
			vertexAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttributeDescs[0].offset = offsetof(vertex, pos);
			// NOTE(heyyod): normal format in vertex shader
			vertexAttributeDescs[1].binding = 0;
			vertexAttributeDescs[1].location = 1;
			vertexAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexAttributeDescs[1].offset = offsetof(vertex, normal);
			// NOTE(heyyod): inTexCoord format in vertex shader
			vertexAttributeDescs[2].binding = 0;
			vertexAttributeDescs[2].location = 2;
			vertexAttributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
			vertexAttributeDescs[2].offset = offsetof(vertex, texCoord);

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;

			if (pipelineID == PIPELINE_MESH)
				vertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(vertexAttributeDescs);
			else
				vertexInputInfo.vertexAttributeDescriptionCount = 1; // ONLY PASS POS FOR WIREFRAME

			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescs;

			VkPipelineShaderStageCreateInfo vsStage = {};
			vsStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vsStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vsStage.module = vs;
			vsStage.pName = "main";

			VkPipelineShaderStageCreateInfo fsStage{};
			fsStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fsStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fsStage.module = fs;
			fsStage.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = {
				vsStage,
				fsStage };
			//------------------------------------------------------------------------
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
			inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

			//------------------------------------------------------------------------
			// NOTE(heyyod): Viewport and scissor are  dynamic and update them with vkCmd commands
			VkPipelineViewportStateCreateInfo viewportInfo = {};
			viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportInfo.viewportCount = 1;
			viewportInfo.scissorCount = 1;

			//------------------------------------------------------------------------
			VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
			rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			if (pipelineID == PIPELINE_MESH)
			{
				rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizerInfo.lineWidth = 1.0f;
			}
			else
			{
				rasterizerInfo.polygonMode = VK_POLYGON_MODE_LINE;
				rasterizerInfo.lineWidth = 0.8f;
			}
			rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			// rasterizerInfo.depthClampEnable = VK_FALSE;
			// rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
			// rasterizerInfo.depthBiasEnable = VK_FALSE;
			// rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
			// rasterizerInfo.depthBiasClamp = 0.0f; // Optional
			// rasterizerInfo.depthBiasSlopeFactor = 0.0f; // Optional

			//------------------------------------------------------------------------
			VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
			multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			// multisamplingInfo.sampleShadingEnable = VK_FALSE;
			multisamplingInfo.rasterizationSamples = vulkanContext->msaaSamples;
			multisamplingInfo.minSampleShading = 1.0f; // Optional
			// multisamplingInfo.pSampleMask = nullptr; // Optional
			// multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
			// multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

			//------------------------------------------------------------------------
			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			// depthStencil.minDepthBounds = 0.0f; // Optional
			// depthStencil.maxDepthBounds = 1.0f; // Optional
			// depthStencil.stencilTestEnable = VK_FALSE;
			// depthStencil.front = {}; // Optional
			// depthStencil.back = {}; // Optional

			//------------------------------------------------------------------------
			VkPipelineColorBlendAttachmentState blendAttachment = {};
			blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachment.blendEnable = VK_FALSE;
			// blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			// blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			// blendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			// blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			// blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			// blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

			VkPipelineColorBlendStateCreateInfo blendingInfo = {};
			blendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blendingInfo.logicOpEnable = VK_FALSE;
			blendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
			blendingInfo.attachmentCount = 1;
			blendingInfo.pAttachments = &blendAttachment;
			// blendingInfo.blendConstants[0] = 0.0f; // Optional
			// blendingInfo.blendConstants[1] = 0.0f; // Optional
			// blendingInfo.blendConstants[2] = 0.0f; // Optional
			// blendingInfo.blendConstants[3] = 0.0f; // Optional

			//------------------------------------------------------------------------
			VkDynamicState dynamicStates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
				// VK_DYNAMIC_STATE_LINE_WIDTH
			};
			VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
			dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateInfo.dynamicStateCount = ArrayCount(dynamicStates);
			dynamicStateInfo.pDynamicStates = dynamicStates;

			//------------------------------------------------------------------------
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
			pipelineInfo.layout = vulkanContext->pipeline[pipelineID].layout;
			pipelineInfo.renderPass = vulkanContext->renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
			pipelineInfo.basePipelineIndex = -1;			  // Optional

			VK_CHECK_RESULT(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &vulkanContext->pipeline[pipelineID].handle));

			vkDestroyShaderModule(vulkanContext->device, vs, 0);
			vkDestroyShaderModule(vulkanContext->device, fs, 0);

			DebugPrintFunctionResult(true);
			return true;
		}
		break;
	}
	return false;
}

static_func bool VulkanCreateAllPipelines()
{
	bool result;
	for (u32 id = 0; id < PIPELINES_COUNT; id++)
	{
		result = VulkanCreatePipeline((VULKAN_PIPELINE_ID)id);
		if (!result)
			break;
	}
	DebugPrint(result);
	Assert(result);
	return (result);
}

FUNC_RENDERER_ON_SHADER_RELOAD(VulkanOnShaderReload)
{
	// TODO: ONLY CREATE NECESARY PIPELINES
	return VulkanCreateAllPipelines();
}

static_func void VulkanClearFrameBuffers()
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
		{
			if (VK_CHECK_HANDLE(vulkanContext->framebuffers[i]))
			{
				vkDestroyFramebuffer(vulkanContext->device, vulkanContext->framebuffers[i], 0);
				// vulkanContext->framebuffers[i] = VK_NULL_HANDLE;
			}
		}
	}
}

static_func bool VulkanCreateFrameBuffers()
{
	VulkanClearFrameBuffers();
	local_var VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = vulkanContext->renderPass;
	framebufferInfo.width = vulkanContext->windowExtent.width;
	framebufferInfo.height = vulkanContext->windowExtent.height;
	framebufferInfo.layers = 1;
	for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
	{
		if (vulkanContext->msaaSamples == VK_SAMPLE_COUNT_1_BIT)
		{
			VkImageView attachments[] = { vulkanContext->swapchainImageViews[i], vulkanContext->depthBuffer.view };
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.attachmentCount = ArrayCount(attachments);
			VK_CHECK_RESULT(vkCreateFramebuffer(vulkanContext->device, &framebufferInfo, 0, &vulkanContext->framebuffers[i]));
		}
		else
		{
			VkImageView attachments[] = { vulkanContext->msaa.view, vulkanContext->depthBuffer.view, vulkanContext->swapchainImageViews[i] };
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.attachmentCount = ArrayCount(attachments);
			VK_CHECK_RESULT(vkCreateFramebuffer(vulkanContext->device, &framebufferInfo, 0, &vulkanContext->framebuffers[i]));
		}
	}
	DebugPrintFunctionResult(true);
	return true;
}

static_func void VulkanClearImage(vulkan_image &img)
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		if (VK_CHECK_HANDLE(img.view))
			vkDestroyImageView(vulkanContext->device, img.view, 0);
		if (VK_CHECK_HANDLE(img.handle))
			vkDestroyImage(vulkanContext->device, img.handle, 0);
		if (VK_CHECK_HANDLE(img.memoryHandle))
			vkFreeMemory(vulkanContext->device, img.memoryHandle, 0);

		img.view = VK_NULL_HANDLE;
		img.handle = VK_NULL_HANDLE;
		img.memoryHandle = VK_NULL_HANDLE;
	}
}

static_func bool VulkanCreateImage(VkImageType type, VkFormat format, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectMask, vulkan_image &imageOut)
{
	VulkanClearImage(imageOut);

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
	VK_CHECK_RESULT(vkCreateImage(vulkanContext->device, &imageInfo, 0, &imageOut.handle));

	VkMemoryRequirements memoryReq = {};
	u32 memIndex = 0;
	vkGetImageMemoryRequirements(vulkanContext->device, imageOut.handle, &memoryReq);
	if (VulkanFindMemoryProperties(memoryReq.memoryTypeBits, memoryProperties, memIndex))
	{
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryReq.size;
		allocInfo.memoryTypeIndex = memIndex;

		VK_CHECK_RESULT(vkAllocateMemory(vulkanContext->device, &allocInfo, 0, &imageOut.memoryHandle));
		VK_CHECK_RESULT(vkBindImageMemory(vulkanContext->device, imageOut.handle, imageOut.memoryHandle, 0));
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
		VK_COMPONENT_SWIZZLE_IDENTITY };
	imageViewInfo.subresourceRange = {
		aspectMask, // VkImageAspectFlags    aspectMask;
		0,			// uint32_t              baseMipLevel;
		mipLevels,	// uint32_t              levelCount;
		0,			// uint32_t              baseArrayLayer;
		1			// uint32_t              layerCount;
	};
	VK_CHECK_RESULT(vkCreateImageView(vulkanContext->device, &imageViewInfo, 0, &imageOut.view));

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

static_func bool VulkanCreateDepthBuffer()
{
	bool result = VulkanCreateImage(VK_IMAGE_TYPE_2D, DEPTH_BUFFER_FORMAT, { vulkanContext->windowExtent.width, vulkanContext->windowExtent.height, 1 },
		1, vulkanContext->msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, vulkanContext->depthBuffer);

	DebugPrintFunctionResult(result);
	return result;
}

static_func bool VulkanCreateMSAABuffer()
{
	if (vulkanContext->msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		VulkanClearImage(vulkanContext->msaa);
		DebugPrintFunctionResult(true);
		return true;
	}

	bool result = VulkanCreateImage(VK_IMAGE_TYPE_2D, vulkanContext->surfaceFormat.format, { vulkanContext->windowExtent.width, vulkanContext->windowExtent.height, 1 },
		1, vulkanContext->msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT, vulkanContext->msaa);

	DebugPrintFunctionResult(result);
	return result;
}

static_func void VulkanClearSwapchainImages()
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		for (u32 i = 0; i < vulkanContext->swapchainImageCount; i++)
		{
			if (VK_CHECK_HANDLE(vulkanContext->swapchainImageViews[i]))
			{
				vkDestroyImageView(vulkanContext->device, vulkanContext->swapchainImageViews[i], 0);
			}
		}
	}
}

static_func bool VulkanCreateSwapchain()
{
	bool result = true;

	if (VK_CHECK_HANDLE(vulkanContext->device))
		vkDeviceWaitIdle(vulkanContext->device);
	else
	{
		result = false;
		DebugPrintFunctionResult(result);
		return false;
	}

	vulkanContext->canRender = false;

	// NOTE: Get Surface capabilities
	{
		VkSurfaceCapabilitiesKHR surfCapabilities = {};
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanContext->gpu, vulkanContext->surface, &surfCapabilities));

		vulkanContext->windowExtent = {};
		if (surfCapabilities.currentExtent.width == UINT32_MAX)
		{
			vulkanContext->windowExtent.width = WINDOW_WIDTH;
			vulkanContext->windowExtent.height = WINDOW_HEIGHT;

			if (vulkanContext->windowExtent.width < surfCapabilities.minImageExtent.width)
				vulkanContext->windowExtent.width = surfCapabilities.minImageExtent.width;
			else if (vulkanContext->windowExtent.width > surfCapabilities.maxImageExtent.width)
				vulkanContext->windowExtent.width = surfCapabilities.maxImageExtent.width;

			if (vulkanContext->windowExtent.height < surfCapabilities.minImageExtent.height)
				vulkanContext->windowExtent.height = surfCapabilities.minImageExtent.height;
			else if (vulkanContext->windowExtent.height > surfCapabilities.maxImageExtent.height)
				vulkanContext->windowExtent.height = surfCapabilities.maxImageExtent.height;
		}
		else
		{ // If the surface size is defined, the swap chain size must match
			vulkanContext->windowExtent = surfCapabilities.currentExtent;
		}
		if ((vulkanContext->windowExtent.width == 0) || (vulkanContext->windowExtent.height == 0))
		{
			return true;
		}

		// NOTE: Determine the number of VkImage's to use in the swap chain.
		// Constant at 2 for now: Double buffering
		Assert(NUM_SWAPCHAIN_IMAGES >= surfCapabilities.minImageCount);
		Assert(NUM_SWAPCHAIN_IMAGES <= surfCapabilities.maxImageCount);
		vulkanContext->swapchainImageCount = NUM_SWAPCHAIN_IMAGES;

		// NOTE: Determine the pre-transform
		VkSurfaceTransformFlagBitsKHR desiredPreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // do nothing
		if (!(surfCapabilities.supportedTransforms & desiredPreTransform))
			desiredPreTransform = surfCapabilities.currentTransform;

		// NOTE: Set the present mode
		VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		// The FIFO present mode is guaranteed by the spec to be supported

		uint32_t presentModeCount;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext->gpu, vulkanContext->surface, &presentModeCount, NULL));
		VkPresentModeKHR presentModes[16] = {};
		;
		Assert(presentModeCount <= ArrayCount(presentModes));
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanContext->gpu, vulkanContext->surface, &presentModeCount, presentModes));
		bool desiredPresentModeSupported = false;
		for (u32 i = 0; i < presentModeCount; i++)
		{
			if (desiredPresentMode == presentModes[i])
				desiredPresentModeSupported = true;
		}
		if (!desiredPresentModeSupported)
		{
			desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR; // VK_PRESENT_MODE_IMMEDIATE_KHR;
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
		VkImageUsageFlags desiredImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // must alwasy be supported
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
		swapchainInfo.surface = vulkanContext->surface;
		swapchainInfo.imageExtent = vulkanContext->windowExtent;
		swapchainInfo.imageFormat = vulkanContext->surfaceFormat.format;
		swapchainInfo.imageColorSpace = vulkanContext->surfaceFormat.colorSpace;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = desiredImageUsageFlags;
		swapchainInfo.minImageCount = vulkanContext->swapchainImageCount;
		swapchainInfo.preTransform = desiredPreTransform;
		swapchainInfo.compositeAlpha = desiredCompositeAlpha;
		swapchainInfo.presentMode = desiredPresentMode;
		swapchainInfo.clipped = true;

		u32 queueFamilyIndices[2] = { vulkanContext->graphicsQueueFamilyIndex, vulkanContext->presentQueueFamilyIndex };
		if (vulkanContext->graphicsQueueFamilyIndex != vulkanContext->presentQueueFamilyIndex)
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

		VkSwapchainKHR oldSwapchain = vulkanContext->swapchain;
		swapchainInfo.oldSwapchain = oldSwapchain;
		VK_CHECK_RESULT(vkCreateSwapchainKHR(vulkanContext->device, &swapchainInfo, 0, &vulkanContext->swapchain));
		if (VK_CHECK_HANDLE(oldSwapchain)) // old swapchain
			vkDestroySwapchainKHR(vulkanContext->device, oldSwapchain, 0);
	}

	// NOTE: Create the Image views
	{
		VulkanClearSwapchainImages();
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(vulkanContext->device, vulkanContext->swapchain, &vulkanContext->swapchainImageCount, 0));
		Assert(vulkanContext->swapchainImageCount == ArrayCount(vulkanContext->swapchainImages));
		VK_CHECK_RESULT(vkGetSwapchainImagesKHR(vulkanContext->device, vulkanContext->swapchain, &vulkanContext->swapchainImageCount, vulkanContext->swapchainImages));

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = vulkanContext->surfaceFormat.format;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		for (u32 i = 0; i < vulkanContext->swapchainImageCount; i++)
		{
			imageViewInfo.image = vulkanContext->swapchainImages[i];
			VK_CHECK_RESULT(vkCreateImageView(vulkanContext->device, &imageViewInfo, 0, &vulkanContext->swapchainImageViews[i]));
		}
	}

	DebugPrintFunctionResult(result);

	if (result)
		result = VulkanCreateDepthBuffer();

	if (result)
		result = VulkanCreateMSAABuffer();

	if (result)
		result = VulkanCreateFrameBuffers();

	DebugPrintSeperator();

	vulkanContext->canRender = result;
	return true;
}

static_func void VulkanClearSwapchain()
{
	VulkanClearImage(vulkanContext->depthBuffer);
	VulkanClearImage(vulkanContext->msaa);
	VulkanClearFrameBuffers();
	VulkanClearSwapchainImages();
	if (VK_CHECK_HANDLE(vulkanContext->swapchain))
		vkDestroySwapchainKHR(vulkanContext->device, vulkanContext->swapchain, 0);
}

FUNC_RENDERER_ON_RESIZE(VulkanOnResize)
{
	bool result = VulkanCreateSwapchain();
	if (result)
	{
		renderer->windowWidth = vulkanContext->windowExtent.width;
		renderer->windowHeight = vulkanContext->windowExtent.height;
	}
	DebugPrintFunctionResult(result);
	return result;
}

static_func bool VulkanCreateBuffer(VkBufferUsageFlags usage, u64 size, VkMemoryPropertyFlags properties, vulkan_buffer &bufferOut, bool mapBuffer = false)
{
	// TODO(heyyod): make this able to create multiple buffers of the same type and usage if I pass an array
	if (VK_CHECK_HANDLE(bufferOut.handle))
		return false;

	bufferOut.size = size;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = usage;
	bufferInfo.size = size;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VK_CHECK_RESULT(vkCreateBuffer(vulkanContext->device, &bufferInfo, 0, &bufferOut.handle));

	VkMemoryRequirements memoryReq = {};
	vkGetBufferMemoryRequirements(vulkanContext->device, bufferOut.handle, &memoryReq);

	u32 memIndex = 0;
	if (VulkanFindMemoryProperties(memoryReq.memoryTypeBits, properties, memIndex))
	{
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryReq.size;
		allocInfo.memoryTypeIndex = memIndex;

		VK_CHECK_RESULT(vkAllocateMemory(vulkanContext->device, &allocInfo, 0, &bufferOut.memoryHandle));
		VK_CHECK_RESULT(vkBindBufferMemory(vulkanContext->device, bufferOut.handle, bufferOut.memoryHandle, 0));

		if (mapBuffer)
			VK_CHECK_RESULT(vkMapMemory(vulkanContext->device, bufferOut.memoryHandle, 0, bufferOut.size, 0, &bufferOut.data));
	}
	else
	{
		Assert("Could not allocate vertex buffer memory");
		return false;
	}

	vulkanContext->buffers[vulkanContext->nextBuffer++] = bufferOut;
	return true;
}

static_func void VulkanClearBuffer(vulkan_buffer buffer)
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		if (VK_CHECK_HANDLE(buffer.handle))
			vkDestroyBuffer(vulkanContext->device, buffer.handle, 0);
		if (VK_CHECK_HANDLE(buffer.memoryHandle))
		{
			vkFreeMemory(vulkanContext->device, buffer.memoryHandle, 0);
		}
	}
}

FUNC_RENDERER_BIND_SHADER_RESOURCES(VulkanBindShaderResources)
{
	VkBufferUsageFlags usage = {};
	VkMemoryPropertyFlags properties = {};
	VkBuffer buffers[10] = {};
	VkDescriptorSetLayoutBinding setBindings[10] = {};
	AssertArraySize(setBindings, count);

	for (u32 i = 0; i < count; i++)
	{
		vulkan_buffer newBuffer = {};
		bool mapFlag = false;

		if (binds[i].type == SHADER_RESOURCE_TYPE_UNIFORM)
		{
			usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			binds[i].size = VulkanGetUniformBufferPaddedSize(binds[i].size);
			
			mapFlag = true;
		}
		else if (binds[i].type == SHADER_RESOURCE_TYPE_STORAGE)
		{
			usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			mapFlag = true;
		}

		u64 size = binds[i].size;
		if (binds[i].perFrame)
			size = binds[i].size * NUM_SWAPCHAIN_IMAGES;

		if (VulkanCreateBuffer(usage, size, properties, newBuffer, mapFlag))
		{
			binds[i].data = newBuffer.data;
			buffers[i] = newBuffer.handle;
		}

		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = binds[i].bind;
		binding.descriptorCount = 1;

		if (binds[i].type == SHADER_RESOURCE_TYPE_UNIFORM)
		{
			/*if (binds[i].perFrame)
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			else*/
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (binds[i].type == SHADER_RESOURCE_TYPE_STORAGE)
		{
			/*if (binds[i].perFrame)
				binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			else*/
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}

		if (binds[i].stages & SHADER_STAGE_FLAG_VERTEX)
			binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (binds[i].stages & SHADER_STAGE_FLAG_FRAGMENT)
			binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (binds[i].stages & SHADER_STAGE_FLAG_COMPUTE)
			binding.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;

		setBindings[i] = binding;
	}


	// TEMP -> DONT FORGET TO CHANGE THE count + 1 below
	//------------------------------------------------------------------------
	setBindings[count].binding = 2;
	setBindings[count].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setBindings[count].descriptorCount = 1;
	setBindings[count].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	//------------------------------------------------------------------------

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = count + 1; // TEMP
	layoutInfo.pBindings = setBindings;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vulkanContext->device, &layoutInfo, 0, &vulkanContext->globalDescriptorSetLayout));

	for (u32 iFrame = 0; iFrame < NUM_SWAPCHAIN_IMAGES; iFrame++)
	{
		VkDescriptorSetAllocateInfo descAlloc = {};
		descAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descAlloc.descriptorPool = vulkanContext->globalDescriptorPool;
		descAlloc.descriptorSetCount = 1;
		descAlloc.pSetLayouts = &vulkanContext->globalDescriptorSetLayout;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkanContext->device, &descAlloc, &vulkanContext->globalDescriptorSet[iFrame]));
	}

	for (u32 i = 0; i < count; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.range = binds[i].size;
		bufferInfo.buffer = buffers[i];

		VkWriteDescriptorSet bufferWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		bufferWrite.dstBinding = binds[i].bind;
		bufferWrite.descriptorType = setBindings[i].descriptorType;
		bufferWrite.descriptorCount = 1;
		bufferWrite.pBufferInfo = &bufferInfo;

		// TODO: This is temporary since I just use a signle desc set (one for each frame in flight).
		for (u32 iFrame = 0; iFrame < NUM_SWAPCHAIN_IMAGES; iFrame++)
		{
			bufferInfo.offset = iFrame * binds[i].size;

			bufferWrite.dstSet = vulkanContext->globalDescriptorSet[iFrame];
			vkUpdateDescriptorSets(vulkanContext->device, 1, &bufferWrite, 0, 0);
		}
	}

	return true;
}

FUNC_RENDERER_REQUEST_BUFFER(VulkanRequestBuffer)
{
	VkBufferUsageFlags usage = {};
	VkMemoryPropertyFlags properties = {};
	bool mapFlag = false;

	void *data = 0;

	if (bufferType == RENDERER_BUFFER_TYPE_UNIFORM || bufferType == RENDERER_BUFFER_TYPE_UNIFORM_DYNAMIC)
	{
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		size = VulkanGetUniformBufferPaddedSize(size);
		mapFlag = true;
	}
	else if (bufferType == RENDERER_BUFFER_TYPE_STORAGE || bufferType == RENDERER_BUFFER_TYPE_STORAGE_DYNAMIC)
	{
		usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		mapFlag = true;
	}
	else if (bufferType == RENDERER_BUFFER_TYPE_STAGING)
	{
		usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		mapFlag = true;
	}

	if (doubleBuffer)
		size *= 2;

	vulkan_buffer newBuffer = {};
	if (VulkanCreateBuffer(usage, size, properties, newBuffer, mapFlag))
	{
		data = newBuffer.data;
	}

	if (bufferType == RENDERER_BUFFER_TYPE_STAGING)
		vulkanContext->stagingBuffer = newBuffer;

	Assert(data);
	return data;
}

static_func vulkan_cmd_resources *VulkanGetNextAvailableResource()
{
	vulkan_cmd_resources *result = &vulkanContext->resources[vulkanContext->currentResource];
	vulkanContext->currentResource = (vulkanContext->currentResource + 1) % NUM_RESOURCES;

	VK_CHECK_RESULT(vkWaitForFences(vulkanContext->device, 1, &result->fence, true, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(vulkanContext->device, 1, &result->fence));

	return result;
}

static_func bool VulkanCreateGrid()
{
	// Create Grid Vertices
	vec3 *v = (vec3 *)vulkanContext->stagingBuffer.data;
	u32 iVert = 0;
	i32 gridSize = 500;
	i32 halfGridSize = gridSize / 2;
	v[iVert++] = { 0.0f, (f32)halfGridSize, 0.0f };
	v[iVert++] = { 0.0f, -(f32)halfGridSize, 0.0f };
	{
		i32 z = halfGridSize;
		for (i32 x = -halfGridSize; x < halfGridSize; x++)
		{
			v[iVert++] = { (f32)x, 0.0f, (f32)z };
			v[iVert++] = { (f32)x, 0.0f, -(f32)z };
		}
	}
	{
		i32 x = halfGridSize;
		for (i32 z = -halfGridSize; z < halfGridSize; z++)
		{
			v[iVert++] = { (f32)x, 0.0f, (f32)z };
			v[iVert++] = { -(f32)x, 0.0f, (f32)z };
		}
	}
	vulkanContext->gridVertexCount = (u32)iVert - 1;
	u32 size = sizeof(vec3) * vulkanContext->gridVertexCount;

	if (!VulkanCreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanContext->gridBuffer))
	{
		Assert("Could not create grid buffer");
		return false;
	}

	vulkan_cmd_resources *res = VulkanGetNextAvailableResource();
	/*
		VkMappedMemoryRange flushRange = {};
		flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		flushRange.memory = vulkanContext->stagingBuffer.memoryHandle;
		flushRange.size = vulkanContext->stagingBuffer.size;
		vkFlushMappedMemoryRanges(vulkanContext->device, 1, &flushRange);
		*/
	VkCommandBufferBeginInfo cmdBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK_RESULT(vkBeginCommandBuffer(res->cmdBuffer, &cmdBufferBeginInfo));

	VkBufferCopy bufferCopyInfo = {};
	bufferCopyInfo.srcOffset = 0;
	bufferCopyInfo.dstOffset = 0;
	bufferCopyInfo.size = size;
	vkCmdCopyBuffer(res->cmdBuffer, vulkanContext->stagingBuffer.handle, vulkanContext->gridBuffer.handle, 1, &bufferCopyInfo);

	VkBufferMemoryBarrier bufferBarriers = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
	bufferBarriers.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	bufferBarriers.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	bufferBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarriers.buffer = vulkanContext->gridBuffer.handle;
	bufferBarriers.offset = 0;
	bufferBarriers.size = size;

	vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, 0, 1, &bufferBarriers, 0, 0);

	VK_CHECK_RESULT(vkEndCommandBuffer(res->cmdBuffer));
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &res->cmdBuffer;
	VK_CHECK_RESULT(vkQueueSubmit(vulkanContext->graphicsQueue, 1, &submitInfo, res->fence));

	return true;
}

FUNC_RENDERER_INITIALIZE(VulkanInitialize)
{
	// TODO: Make it cross-platform
	DebugPrint("INITIALIZING vulkanContext->..\n");

	bool result;

	result = VulkanLoadCode();

	if (result)
		result = VulkanLoadGlobalFunctions();

	if (result)
		result = VulkanCreateInstance(renderer, appName);

	if (result)
		result = VulkanCreateDevice(renderer);

	if (result)
		result = VulkanCreateCommandResources();

	//------------------------------------------------------------------------
	// CREATE BUFFERS
	//------------------------------------------------------------------------
	{
		u32 vertexBufferSize = MEGABYTES(64);
		u32 indexBufferSize = MEGABYTES(64);

		if (!VulkanCreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanContext->vertexBuffer))
		{
			Assert("Could not create vertex buffer");
			return false;
		}

		if (!VulkanCreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanContext->indexBuffer))
		{
			Assert("Could not create index buffer");
			return false;
		}
	}

	//------------------------------------------------------------------------
	// CREATE TEXTURE SAMPLER
	//------------------------------------------------------------------------
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
		samplerInfo.maxAnisotropy = vulkanContext->gpuProperties.limits.maxSamplerAnisotropy;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 11.0f; // TODO(heyyod): THIS IS HARDCODED AND BAD
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		VK_CHECK_RESULT(vkCreateSampler(vulkanContext->device, &samplerInfo, 0, &vulkanContext->textureSampler));
	}

	//------------------------------------------------------------------------
	// CREATE DESCRIPTORS
	//------------------------------------------------------------------------
	/**
	 * WHAT THE FUCK IS UP WITH DESCRIPTORS:
	 * 
	 * - So we have a shader that we need to pass to is certain data/resources. This data is located
	 *   in a buffer that we have allocated. This buffer is mapped so that the CPU can see it and write
	 *   into it. BUT just writing out data there doen't mean shit.
	 * 
	 * - What we need to do is "declare" the resource that we are going to pass to a shader and basicaly
	 *   point to where it's at. In vulkan, this is done by a descriptor (for each resource) when calling
	 *   vkUpdateDescriptorSets. The info we pass basicaly say: here is the buffer I have, here is its
	 *   type, here is the binding it's gonna be read from, and here is the set it is grouped in.
	 * 
	 * - But wait. What the fuck is a set?
	 *   Well, we basicaly never use a single descriptor as it is apart from creating it and binding it
	 *   to the actual buffer where we write our data. We always group them in descriptor sets.
	 * 
	 * - Now these descriptor sets are created based on a descriptor set layout that we have declared.
	 *   This layout contains all the resource bindings that are used in a shader. So basically a single
	 *   binding in a set matches a single descriptor that we are later going to bind to a piece of memory.
	 *
	 * - From a layout we can create multiple descriptor sets. But the creation of descriptor sets is done
	 *   through a descriptor pool. We use that pool to say to vulkan the number of descriptors it's allowed
	 *   to allocate for each type. Then we allocate a descriptor which returns a handle. This is the handle
	 *   we use to bind the sets before the draw commands.
	 * 
	 * - But a layout is also used in another important way. When creating a pipeline we specify an array
	 *   of descriptor sets layouts that is uses. This basicaly tells the pipeline what kind of data to
	 *   expext when we actually use it (by binding it before a draw call).
	 * 
	 * - So the commands look like this:
	 *     1. Bind the pipeline you want
	 *     2. Bind the descriptor sets you want
	 *     3. Draw 
	 * 
	 * - PS. We usually seperate descriptors into sets depending on the resource usage frequency. This is
	 *   because the binding of a set is done before a draw call and is generally an expensive operation.
	 *   So if for less frequently used resources we do less bindings it's better.
	 *   OBVIOULSY IT'S MORE COMPLIATED.
	 * 
	 */

	{
		// Make a pool
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = ArrayCount(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 5;
		VK_CHECK_RESULT(vkCreateDescriptorPool(vulkanContext->device, &poolInfo, 0, &vulkanContext->globalDescriptorPool));
	}

	VulkanPickMSAA(START_UP_MSAA);

	if (result)
		result = VulkanCreateRenderPass();

	/* NOTE(heyyod): It looks like window always sends a WM_SIZE message
		at startup. This Recreats the following. So we don't need to create these here
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

	vulkanContext->canRender = result;
	DebugPrintFunctionResult(result);
	DebugPrintSeperator();
	return result;

}

inline static_func void VulkanDestroy()
{
	if (VK_CHECK_HANDLE(vulkanContext->device))
	{
		vkDeviceWaitIdle(vulkanContext->device);

		if (VK_CHECK_HANDLE(vulkanContext->textureSampler))
			vkDestroySampler(vulkanContext->device, vulkanContext->textureSampler, 0);

		VulkanClearImage(vulkanContext->texture);

		for (u32 id = 0; id < PIPELINES_COUNT; id++)
		{
			VulkanClearPipeline((VULKAN_PIPELINE_ID)id);
		}

		if (VK_CHECK_HANDLE(vulkanContext->globalDescriptorSetLayout))
			vkDestroyDescriptorSetLayout(vulkanContext->device, vulkanContext->globalDescriptorSetLayout, 0);

		for (u32 i = 0; i < vulkanContext->nextBuffer; i++)
			VulkanClearBuffer(vulkanContext->buffers[i]);

		if (VK_CHECK_HANDLE(vulkanContext->globalDescriptorPool))
			vkDestroyDescriptorPool(vulkanContext->device, vulkanContext->globalDescriptorPool, 0);

		vkUnmapMemory(vulkanContext->device, vulkanContext->stagingBuffer.memoryHandle);
		//VulkanClearBuffer(vulkanContext->stagingBuffer);
		VulkanClearBuffer(vulkanContext->vertexBuffer);
		VulkanClearBuffer(vulkanContext->indexBuffer);
		VulkanClearBuffer(vulkanContext->gridBuffer);

		VulkanClearRenderPass();

		VulkanClearSwapchain();

		for (u32 i = 0; i < NUM_RESOURCES; i++)
		{
			if (VK_CHECK_HANDLE(vulkanContext->resources[i].imgAvailableSem))
				vkDestroySemaphore(vulkanContext->device, vulkanContext->resources[i].imgAvailableSem, 0);
			if (VK_CHECK_HANDLE(vulkanContext->resources[i].frameReadySem))
				vkDestroySemaphore(vulkanContext->device, vulkanContext->resources[i].frameReadySem, 0);
			if (VK_CHECK_HANDLE(vulkanContext->resources[i].fence))
				vkDestroyFence(vulkanContext->device, vulkanContext->resources[i].fence, 0);
		}

		if (VK_CHECK_HANDLE(vulkanContext->cmdPool))
			vkDestroyCommandPool(vulkanContext->device, vulkanContext->cmdPool, 0);

		vkDestroyDevice(vulkanContext->device, 0);
	}

	if (VK_CHECK_HANDLE(vulkanContext->instance))
	{
		if (VK_CHECK_HANDLE(vulkanContext->surface))
			vkDestroySurfaceKHR(vulkanContext->instance, vulkanContext->surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
		if (VK_CHECK_HANDLE(vulkanContext->debugMessenger))
		vkDestroyDebugUtilsMessengerEXT(vulkanContext->instance, vulkanContext->debugMessenger, 0);
#endif

		vkDestroyInstance(vulkanContext->instance, 0);
	}

	if (vulkanContext->dll)
		FreeLibrary(vulkanContext->dll);

	DebugPrintFunctionResult(true);
}

FUNC_RENDERER_CHANGE_GRAPHICS_SETTINGS(VulkanChangeGraphicsSettings)
{
	if (newSettings & CHANGE_MSAA)
	{
		VulkanPickMSAA(settings.msaa);
		bool result = VulkanCreateRenderPass();

		if (result)
			result = VulkanCreateDepthBuffer();

		if (result)
			result = VulkanCreateMSAABuffer();

		if (result)
			result = VulkanCreateFrameBuffers();

		if (result)
		{
			for (u32 id = 0; id < PIPELINES_COUNT; id++)
			{
				result = VulkanCreatePipeline((VULKAN_PIPELINE_ID)id);
				if (!result)
					break;
			}
		}

		DebugPrintFunctionResult(result);
		Assert(result);
		return result;
	}
	return true;
}

static_func void VulkanCmdChangeImageLayout(VkCommandBuffer cmdBuffer, VkImage imgHandle, image *imageInfo, VkImageLayout oldLayout, VkImageLayout newLayout)
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

FUNC_RENDERER_UPLOAD_RESOURCES(VulkanUpload)
{
	vulkan_cmd_resources *res = VulkanGetNextAvailableResource();

	VkMappedMemoryRange flushRange = {};
	flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	flushRange.memory = vulkanContext->stagingBuffer.memoryHandle;
	flushRange.size = vulkanContext->stagingBuffer.size;
	vkFlushMappedMemoryRanges(vulkanContext->device, 1, &flushRange);

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK_RESULT(vkBeginCommandBuffer(res->cmdBuffer, &cmdBufferBeginInfo));

	bool pushedMesh = false;
	bool pushedImage = false;

	u64 vertexBufferStartOffset = vulkanContext->vertexBuffer.writeOffset;
	u64 indexBufferStartOffset = vulkanContext->indexBuffer.writeOffset;

	for (u32 resourceId = 0; resourceId < staged->count; resourceId++)
	{

		switch (staged->types[resourceId])
		{
			//-
			case RESOURCE_MESH:
			{
				pushedMesh = true;
				mesh *m = (mesh *)staged->resources[resourceId];

				VkBufferCopy bufferCopyInfo = {};
				// NOTE(heyyod): Set where to read and where to copy the indices
				bufferCopyInfo.srcOffset = staged->offsets[resourceId];
				bufferCopyInfo.dstOffset = vulkanContext->indexBuffer.writeOffset;
				bufferCopyInfo.size = MESH_PTR_INDICES_SIZE(m);
				vulkanContext->indexBuffer.writeOffset += bufferCopyInfo.size; // set next write offset in index buffer
				vkCmdCopyBuffer(res->cmdBuffer, vulkanContext->stagingBuffer.handle, vulkanContext->indexBuffer.handle, 1, &bufferCopyInfo);

				// NOTE(heyyod): Set where to read and where to copy the vertices
				bufferCopyInfo.srcOffset = bufferCopyInfo.srcOffset + bufferCopyInfo.size;
				bufferCopyInfo.dstOffset = vulkanContext->vertexBuffer.writeOffset;
				bufferCopyInfo.size = MESH_PTR_VERTICES_SIZE(m);
				vulkanContext->vertexBuffer.writeOffset += bufferCopyInfo.size; // set next write offset in vertex buffer
				vkCmdCopyBuffer(res->cmdBuffer, vulkanContext->stagingBuffer.handle, vulkanContext->vertexBuffer.handle, 1, &bufferCopyInfo);

				vulkanContext->loadedMesh[vulkanContext->loadedMeshCount].nIndices = m->nIndices;
				vulkanContext->loadedMesh[vulkanContext->loadedMeshCount].nVertices = m->nVertices;
				vulkanContext->loadedMesh[vulkanContext->loadedMeshCount].nInstances = staged->nInstances[resourceId];
				vulkanContext->loadedMesh[vulkanContext->loadedMeshCount].pipelineID = PIPELINE_MESH;

				// u32 objectTransformsSize = staged->nInstances[resourceId] * sizeof(object_transform);
				// void *dst = vulkanContext->transformsStorage.data;
				// AdvancePointer(dst, vulkanContext->transformsStorage.writeOffset);

				// vulkanContext->loadedMesh[vulkanContext->loadedMeshCount].transforms = dst - vulkanContext->transformsStorage.data;

				// memcpy(dst, staged->transforms, objectTransformsSize);

				vulkanContext->loadedMeshCount++;
			}
			break;

			case RESOURCE_TEXTURE:
			{
				pushedImage = true;
				image *img = (image *)staged->resources[resourceId];
				if (!VulkanCreateImage(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { img->width, img->height, 1 }, img->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT |	 // to create the mipmaps
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | // to copy buffer into image
					VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					VK_IMAGE_ASPECT_COLOR_BIT,
					vulkanContext->texture))
				{
					return false;
				}

				// NOTE(heyyod): We need to write into the image from the stage buffer.
				// So we change the layout.
				{
					VulkanCmdChangeImageLayout(res->cmdBuffer, vulkanContext->texture.handle, img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

					VkBufferImageCopy bufferToImageCopy = {};
					bufferToImageCopy.bufferOffset = staged->offsets[resourceId];
					bufferToImageCopy.bufferRowLength = 0;
					bufferToImageCopy.bufferImageHeight = 0;
					bufferToImageCopy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
					bufferToImageCopy.imageOffset = { 0, 0, 0 };
					bufferToImageCopy.imageExtent = { img->width, img->height, 1 };
					vkCmdCopyBufferToImage(res->cmdBuffer, vulkanContext->stagingBuffer.handle, vulkanContext->texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferToImageCopy);
				}

				// NOTE(heyyod): Generate mipmaps
				// NOTE(heyyod): Check if image format supports linear blitting
				VkFormatProperties formatProperties;
				vkGetPhysicalDeviceFormatProperties(vulkanContext->gpu, VK_FORMAT_R8G8B8A8_UNORM, &formatProperties);
				if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
				{
					i32 mipWidth = (i32)img->width;
					i32 mipHeight = (i32)img->height;

					VkImageMemoryBarrier sourceMipBarrier{};
					sourceMipBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					sourceMipBarrier.image = vulkanContext->texture.handle;
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
							vulkanContext->texture.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanContext->texture.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
					/**
					* There are two alternatives in this case. You could implement a func
					* that searches common texture image formats for one that does support linear
					* blitting, or you could implement the mipmap generation in software with a library
					* like stb_image_resize. Each mip level can then be loaded into the image in the same
					* way that you loaded the original image.
					* It should be noted that it is uncommon in practice to generate the mipmap levels at
					* runtime anyway. Usually they are pregenerated and stored in the texture file alongside
					* the base level to improve loading speed. Implementing resizing in software and loading
					* multiple levels from a file is left as an exercise to the reader.
					*/
				}

				// NOTE(heyyod): Since it will be used in shaders we again need to change the layout
				// CmdChangeImageLayout(res->cmdBuffer, vulkanContext->texture.handle, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				// NOTE(heyyod): Update the descriptor set
				{
					VkDescriptorImageInfo descImageInfo = {};
					descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					descImageInfo.imageView = vulkanContext->texture.view;
					descImageInfo.sampler = vulkanContext->textureSampler;

					VkWriteDescriptorSet writeDesc = {};
					writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDesc.dstBinding = 2;
					writeDesc.dstArrayElement = 0;
					writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writeDesc.descriptorCount = 1;
					writeDesc.pImageInfo = &descImageInfo;

					for (u32 iFrame = 0; iFrame < NUM_SWAPCHAIN_IMAGES; iFrame++)
					{
						writeDesc.dstSet = vulkanContext->globalDescriptorSet[iFrame];
						vkUpdateDescriptorSets(vulkanContext->device, 1, &writeDesc, 0, 0);
					}
				}
			}
			break;

			default:
			{
				DebugPrint("ERROR: Trying to push unknown resource ata type.\n");
				// return false;
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
		bufferBarriers[0].buffer = vulkanContext->vertexBuffer.handle;
		bufferBarriers[0].offset = vertexBufferStartOffset;
		bufferBarriers[0].size = vulkanContext->vertexBuffer.writeOffset - vertexBufferStartOffset;

		bufferBarriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarriers[1].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		bufferBarriers[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
		bufferBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferBarriers[1].buffer = vulkanContext->indexBuffer.handle;
		bufferBarriers[1].offset = indexBufferStartOffset;
		bufferBarriers[1].size = vulkanContext->indexBuffer.writeOffset - indexBufferStartOffset;

		vkCmdPipelineBarrier(res->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, 0, ArrayCount(bufferBarriers), bufferBarriers, 0, 0);
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(res->cmdBuffer));
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &res->cmdBuffer;
	VK_CHECK_RESULT(vkQueueSubmit(vulkanContext->graphicsQueue, 1, &submitInfo, res->fence));

	return true;
}

FUNC_RENDERER_DRAW_FRAME(VulkanDraw)
{
	//------------------------------------------------------------------------
	// BEGIN FRAME
	//------------------------------------------------------------------------
	vulkan_cmd_resources *res = VulkanGetNextAvailableResource();

	// NOTE: Get the next image that we'll use to create the frame and draw it
	// Signal the semaphore when it's available
	local_var u32 nextImage = 0;
	local_var VkResult result = {};
	{
		result = vkAcquireNextImageKHR(vulkanContext->device, vulkanContext->swapchain, UINT64_MAX, res->imgAvailableSem, 0, &nextImage);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!VulkanCreateSwapchain())
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
		clearValues[0].color = { packet->clearColor[0], packet->clearColor[1], packet->clearColor[2], 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		local_var VkRenderPassBeginInfo renderpassInfo = {};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = vulkanContext->renderPass;
		renderpassInfo.renderArea.extent = vulkanContext->windowExtent;
		renderpassInfo.clearValueCount = ArrayCount(clearValues);
		renderpassInfo.pClearValues = clearValues;
		renderpassInfo.framebuffer = vulkanContext->framebuffers[nextImage];

		local_var VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (f32)vulkanContext->windowExtent.height;
		viewport.width = (f32)vulkanContext->windowExtent.width;
		viewport.height = -(f32)vulkanContext->windowExtent.height;
		viewport.maxDepth = 1.0f;

		local_var VkRect2D scissor = {};
		scissor.extent = vulkanContext->windowExtent;

		VK_CHECK_RESULT(vkBeginCommandBuffer(res->cmdBuffer, &commandBufferBeginInfo));
		vkCmdBeginRenderPass(res->cmdBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(res->cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(res->cmdBuffer, 0, 1, &scissor);
	}

	//------------------------------------------------------------------------
	// ISSUE DRAW COMMANDS
	//------------------------------------------------------------------------
	{
		local_var VkDeviceSize bufferOffset = 0;
		vkCmdBindVertexBuffers(res->cmdBuffer, 0, 1, &vulkanContext->vertexBuffer.handle, &bufferOffset);
		vkCmdBindIndexBuffer(res->cmdBuffer, vulkanContext->indexBuffer.handle, 0, VULKAN_INDEX_TYPE);
		u32 firstIndex = 0;
		u32 indexOffset = 0;
		i32 currentPipelineId = -1;
		u32 transformId = 0;

		/*
		u32 uboOffsets[] = {
			(u32)(nextImage * vulkanContext->cameraUBO.paddedSize),
			(u32)(nextImage * vulkanContext->sceneUBO.paddedSize),
			(u32)(nextImage * (vulkanContext->transformsStorage.size / NUM_SWAPCHAIN_IMAGES))
		};
		*/

		for (u32 meshId = 0; meshId < vulkanContext->loadedMeshCount; meshId++)
		{
			if (currentPipelineId != (i32)vulkanContext->loadedMesh[meshId].pipelineID)
			{
				currentPipelineId = vulkanContext->loadedMesh[meshId].pipelineID;
				vkCmdBindPipeline(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].handle);
				vkCmdBindDescriptorSets(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].layout, 0, 1, &vulkanContext->globalDescriptorSet[nextImage], 0, 0);
			}
			for (u32 meshInstance = 0; meshInstance < vulkanContext->loadedMesh[meshId].nInstances; meshInstance++)
			{
				vkCmdPushConstants(res->cmdBuffer, vulkanContext->pipeline[currentPipelineId].layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(u32), &transformId);
				vkCmdDrawIndexed(res->cmdBuffer, vulkanContext->loadedMesh[meshId].nIndices, 1, firstIndex, indexOffset, 0);

				// NOTE(heyyod): THIS ASSUMES THE TRANFORMS ARE ALWAYS LINEARLY SAVED :(
				transformId++;
			}

			// WIREFRAME ------------------------------------------------------------
			transformId = 0;
			currentPipelineId = PIPELINE_WIREFRAME;
			vkCmdBindPipeline(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].handle);
			vkCmdBindDescriptorSets(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].layout, 0, 1, &vulkanContext->globalDescriptorSet[nextImage], 0, 0);
			for (u32 meshInstance = 0; meshInstance < vulkanContext->loadedMesh[meshId].nInstances; meshInstance++)
			{
				vkCmdPushConstants(res->cmdBuffer, vulkanContext->pipeline[currentPipelineId].layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(u32), &transformId);
				vkCmdDrawIndexed(res->cmdBuffer, vulkanContext->loadedMesh[meshId].nIndices, 1, firstIndex, indexOffset, 0);
				transformId++;
			}
			//------------------------------------------------------------------------

			firstIndex += vulkanContext->loadedMesh[meshId].nIndices;
			indexOffset += vulkanContext->loadedMesh[meshId].nVertices;
		}

		currentPipelineId = PIPELINE_GRID;
		vkCmdBindVertexBuffers(res->cmdBuffer, 0, 1, &vulkanContext->gridBuffer.handle, &bufferOffset);
		vkCmdBindPipeline(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].handle);
		vkCmdBindDescriptorSets(res->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanContext->pipeline[currentPipelineId].layout, 0, 1, &vulkanContext->globalDescriptorSet[nextImage], 0, 0);
		vkCmdPushConstants(res->cmdBuffer, vulkanContext->pipeline[currentPipelineId].layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec3), &packet->playerPos);
		vkCmdDraw(res->cmdBuffer, vulkanContext->gridVertexCount, 1, 0, 0);

		vkCmdEndRenderPass(res->cmdBuffer);
		VK_CHECK_RESULT(vkEndCommandBuffer(res->cmdBuffer));
	}

	//------------------------------------------------------------------------
	// END FRAME
	//------------------------------------------------------------------------
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
		VK_CHECK_RESULT(vkQueueSubmit(vulkanContext->graphicsQueue, 1, &submitInfo, res->fence));

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (!VulkanCreateSwapchain())
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
		presentInfo.pSwapchains = &vulkanContext->swapchain;
		presentInfo.pImageIndices = &nextImage;
		result = vkQueuePresentKHR(vulkanContext->presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			if (!VulkanCreateSwapchain())
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

	// UPDATE RENDER PACKET FOR ENGINE
	{
		// VEEEEEEERY TEMPORARY
		// !!!!!!!!!!! DANGER !!!!!!!!!!!!!!!!!
		u32 frameID = (nextImage + 1) % NUM_SWAPCHAIN_IMAGES;
		packet->nextCameraPtr = (void *)((u8 *)vulkanContext->buffers[2].data + frameID * (vulkanContext->buffers[2].size / NUM_SWAPCHAIN_IMAGES));
		packet->nextScenePtr = (void *)((u8 *)vulkanContext->buffers[3].data + frameID * (vulkanContext->buffers[3].size / NUM_SWAPCHAIN_IMAGES));
		packet->nextTransformsPtr = (void *)((u8 *)vulkanContext->buffers[4].data + frameID * (vulkanContext->buffers[4].size / NUM_SWAPCHAIN_IMAGES));
	}
	return true;
}

#if 0
static_func bool VulkanCreateCommandBuffers()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	vkCmdBindDescriptorSets()
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkanContext->presentQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(vulkanContext->device, &cmdPoolInfo, 0, &vulkanContext->cmdPool));

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = vulkanContext->cmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;
	for (u32 i = 0; i < NUM_RESOURCES; i++)
	{
		VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkanContext->device, &cmdBufferAllocInfo, &vulkanContext->resources[i].cmdBuffer));
	}
	DebugPrint("Created Command Pool and Command Buffers\n");

	return true;
}


static_func bool VulkanResetCommandBuffers()
{
	if (VK_CHECK_HANDLE(vulkanContext->device) && VK_CHECK_HANDLE(vulkanContext->cmdPool))
	{
		vkDeviceWaitIdle(vulkanContext->device);
		vkResetCommandPool(vulkanContext->device, vulkanContext->cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		DebugPrint("Reseted Command Buffers\n");
		return true;
	}
	return false;
}

static_func void VulkanGetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc)
{
	bindingDesc.binding = 0;
	bindingDesc.stride = sizeof(v);
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

static_func void VulkanGetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs)
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
