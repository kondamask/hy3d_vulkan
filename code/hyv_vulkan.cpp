#include "hyv_vulkan.h"

// TODO: make this cross-platform
static bool Win32LoadVulkanDLL(vulkan_state &vulkan)
{
    vulkan.dll = LoadLibraryA("vulkan-1.dll");
    if (!vulkan.dll)
        return false;

    vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkan.dll, "vkGetInstanceProcAddr");
    return true;
}

#define VulkanLoadGlobalFunc(func)                                     \
    func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(nullptr, #func); \
    if (!(func))                                                       \
    {                                                                  \
        return false;                                                  \
    }
static bool VulkanLoadGlobalFunctions()
{
    VulkanLoadGlobalFunc(vkCreateInstance)
        VulkanLoadGlobalFunc(vkEnumerateInstanceLayerProperties) return true;
}

#define VulkanLoadInstanceFunc(instance, func)                          \
    func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(instance, #func); \
    if (!(func))                                                        \
    {                                                                   \
        return false;                                                   \
    }
static bool VulkanLoadInstanceFunctions(VkInstance instance)
{
    VulkanLoadInstanceFunc(instance, vkDestroyInstance);

    VulkanLoadInstanceFunc(instance, vkCreateDebugUtilsMessengerEXT);
    VulkanLoadInstanceFunc(instance, vkDestroyDebugUtilsMessengerEXT);

    VulkanLoadInstanceFunc(instance, vkCreateWin32SurfaceKHR); // NOTE: Windows ONLY
    VulkanLoadInstanceFunc(instance, vkDestroySurfaceKHR);

    VulkanLoadInstanceFunc(instance, vkEnumeratePhysicalDevices);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceFormatProperties);
    VulkanLoadInstanceFunc(instance, vkGetPhysicalDeviceMemoryProperties);

    VulkanLoadInstanceFunc(instance, vkEnumerateDeviceExtensionProperties);
    VulkanLoadInstanceFunc(instance, vkCreateDevice);
    VulkanLoadInstanceFunc(instance, vkGetDeviceProcAddr);

    return true;
}

#define VulkanLoadDeviceFunc(device, func)                          \
    func = (VulkanFuncPtr(func))vkGetDeviceProcAddr(device, #func); \
    if (!(func))                                                    \
    {                                                               \
        return false;                                               \
    }
static bool VulkanLoadDeviceFunctions(VkDevice device)
{
    VulkanLoadDeviceFunc(device, vkDeviceWaitIdle);
    VulkanLoadDeviceFunc(device, vkDestroyDevice);
    VulkanLoadDeviceFunc(device, vkGetDeviceQueue);

    VulkanLoadDeviceFunc(device, vkCreateSwapchainKHR);
    VulkanLoadDeviceFunc(device, vkDestroySwapchainKHR);
    VulkanLoadDeviceFunc(device, vkGetSwapchainImagesKHR);

    VulkanLoadDeviceFunc(device, vkCreateCommandPool);
    VulkanLoadDeviceFunc(device, vkDestroyCommandPool);
    VulkanLoadDeviceFunc(device, vkAllocateCommandBuffers);
    VulkanLoadDeviceFunc(device, vkBeginCommandBuffer);
    VulkanLoadDeviceFunc(device, vkEndCommandBuffer);
    VulkanLoadDeviceFunc(device, vkCmdPipelineBarrier);
    VulkanLoadDeviceFunc(device, vkCmdClearColorImage);

    VulkanLoadDeviceFunc(device, vkCreateImage);
    VulkanLoadDeviceFunc(device, vkDestroyImage);
    VulkanLoadDeviceFunc(device, vkCreateImageView);
    VulkanLoadDeviceFunc(device, vkDestroyImageView);
    VulkanLoadDeviceFunc(device, vkGetImageMemoryRequirements);
    VulkanLoadDeviceFunc(device, vkBindImageMemory);

    VulkanLoadDeviceFunc(device, vkAllocateMemory);
    VulkanLoadDeviceFunc(device, vkFreeMemory);
    return true;
}

static bool VulkanFindMemoryProperties(VkPhysicalDeviceMemoryProperties &memoryProperties,
                                       uint32_t memoryTypeBitsRequirement,
                                       VkMemoryPropertyFlags requiredProperties,
                                       u32 &memoryIndex)
{
    u32 memoryCount = memoryProperties.memoryTypeCount;
    for (memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
    {
        uint32_t memoryTypeBits = (1 << memoryIndex);
        bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;
        VkMemoryPropertyFlags properties = memoryProperties.memoryTypes[memoryIndex].propertyFlags;
        bool hasRequiredProperties = ((properties & requiredProperties) == requiredProperties);

        if (isRequiredMemoryType && hasRequiredProperties)
            return true;
    }
    return false;
}

#if VULKAN_VALIDATION_LAYERS_ON
#include <iostream>
static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
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
        HYV_DEBUG_PRINT("WARNING: ");
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        HYV_DEBUG_PRINT("DIAGNOSTIC: ");
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        HYV_DEBUG_PRINT("INFO: ");
    }
    else
    {
        HYV_DEBUG_PRINT("ERROR: ");
        isError = true;
    }
    HYV_DEBUG_PRINT(type);
    HYV_DEBUG_PRINT('\n');
    HYV_DEBUG_PRINT(pCallbackData->pMessageIdName);
    HYV_DEBUG_PRINT('\n');

    char c;
    u32 i = 0;
    do
    {
        c = pCallbackData->pMessage[i];
        i++;
    } while (c != ']');
    i++;
    do
    {
        c = pCallbackData->pMessage[i];
        if (c != ';' && c != '|')
        {
            HYV_DEBUG_PRINT(c);
            i++;
        }
        else
        {
            HYV_DEBUG_PRINT('\n');
            i++;
            do
            {
                c = pCallbackData->pMessage[i];
                i++;
            } while (c == ' ' || c == '|');
            i--;
        }
    } while (c != '\0');
    HYV_DEBUG_PRINT('\n');
    if (isError)
    {
        Assert(0);
        return VK_FALSE;
    }
    return VK_FALSE;
}
#endif

// TODO: Make it cross-platform
static bool Win32InitializeVulkan(vulkan_state &vulkan, HINSTANCE &wndInstance, HWND &wndHandle, const char *name, u16 width, u16 height)
{
    Assert(VulkanLoadGlobalFunctions());

#if VULKAN_VALIDATION_LAYERS_ON
    // NOTE: Enable Validation Layer
    const bool enableValidationLayers = true;

    char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"};

    u32 layerCount;
    ASSERT_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&layerCount, 0));

    VkLayerProperties availableLayers[16];
    Assert(layerCount <= ArrayCount(availableLayers));
    ASSERT_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

    for (char *layerName : validationLayers)
    {
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
            Assert("ERROR: Validation Layer not found");
            return false;
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = VulkanDebugCallback;

    char *instanceExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#else
    const bool enableValidationLayers = false;

    char *instanceExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#endif

    // NOTE: Set application info (optionl but usefule)
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // NOTE: Create an instance and attach extensions
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = ArrayCount(instanceExtensions);
    instanceInfo.ppEnabledExtensionNames = instanceExtensions;
#if VULKAN_VALIDATION_LAYERS_ON
    instanceInfo.enabledLayerCount = ArrayCount(validationLayers);
    instanceInfo.ppEnabledLayerNames = validationLayers;
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif

    ASSERT_VK_SUCCESS(vkCreateInstance(&instanceInfo, NULL, &vulkan.instance));
    Assert(VulkanLoadInstanceFunctions(vulkan.instance));

#if VULKAN_VALIDATION_LAYERS_ON
    ASSERT_VK_SUCCESS(vkCreateDebugUtilsMessengerEXT(vulkan.instance, &debugMessengerInfo, 0, &vulkan.debugMessenger));
#endif

    // NOTE: Create a surface
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.hinstance = wndInstance;
        surfaceInfo.hwnd = wndHandle;
        ASSERT_VK_SUCCESS(vkCreateWin32SurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));
    }

    // NOTE: Select a gpu to use
    {
        u32 gpuCount = 0;
        ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, 0));

        VkPhysicalDevice gpuBuffer[16] = {};
        Assert(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
        ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, gpuBuffer));
        vulkan.gpu = gpuBuffer[0];
        // TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
        //For now it's ok since I only have 1 gpu.

        vkGetPhysicalDeviceMemoryProperties(vulkan.gpu, &vulkan.memoryProperties);
    }

    // NOTE: Pick a queue family the supports both present and graphics operations
    u32 graphicsQueueFamilyIndex = UINT32_MAX;
    u32 presentQueueFamilyIndex = UINT32_MAX;
    {
        u32 queueFamilyCount;
        VkQueueFamilyProperties availableQueueFamilies[16] = {};
        VkBool32 supportsPresent[16] = {};
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, 0);
        Assert(queueFamilyCount <= ArrayCount(availableQueueFamilies));
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, availableQueueFamilies);

        for (u32 i = 0; i < queueFamilyCount; i++)
            ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfaceSupportKHR(vulkan.gpu, i, vulkan.surface, &supportsPresent[i]));

        for (u32 i = 0; i < queueFamilyCount; ++i)
        {
            if ((availableQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                if (graphicsQueueFamilyIndex == UINT32_MAX)
                    graphicsQueueFamilyIndex = i;
                if (supportsPresent[i] == VK_TRUE)
                {
                    graphicsQueueFamilyIndex = i;
                    presentQueueFamilyIndex = i;
                    break;
                }
            }
        }
        if (presentQueueFamilyIndex == UINT32_MAX) //didn't find a queue that supports both graphics and present
        {
            for (u32 i = 0; i < queueFamilyCount; ++i)
                if (supportsPresent[i] == VK_TRUE)
                {
                    presentQueueFamilyIndex = i;
                    break;
                }
        }
        if (graphicsQueueFamilyIndex == UINT32_MAX)
        {
            Assert("ERROR: No graphics queue found.\n");
            return false;
        }
        if (presentQueueFamilyIndex == UINT32_MAX)
        {
            Assert("ERROR: No present queue found.\n");
            return false;
        }
    }

    Assert(graphicsQueueFamilyIndex == presentQueueFamilyIndex);
    vulkan.queueFamilyIndex = presentQueueFamilyIndex;
    // TODO: Neet to do some stuff if they are different like:

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
        queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        char *desiredDeviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        u32 deviceExtensionsCount;
        VkExtensionProperties availableDeviceExtensions[255];
        ASSERT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, 0));
        Assert(deviceExtensionsCount <= ArrayCount(availableDeviceExtensions));
        ASSERT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, availableDeviceExtensions));
        for (char *desiredExtension : desiredDeviceExtensions)
        {
            bool found = false;
            for (VkExtensionProperties &availableExtension : availableDeviceExtensions)
            {
                if (strcmp(desiredExtension, availableExtension.extensionName) == 0)
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

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = ArrayCount(desiredDeviceExtensions);
        deviceInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
        ASSERT_VK_SUCCESS(vkCreateDevice(vulkan.gpu, &deviceInfo, 0, &vulkan.device));
        Assert(VulkanLoadDeviceFunctions(vulkan.device));
    }

    // NOTE: Get the queue and save it into our vulkan object
    vkGetDeviceQueue(vulkan.device, vulkan.queueFamilyIndex, 0, &vulkan.queue);

    // NOTE: Create a swapchain

    // NOTE: 1.set a proper surface format
    {
        vulkan.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        vulkan.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        u32 formatCount = 0;
        VkSurfaceFormatKHR availableFormats[16] = {};
        bool desiredSurfaceFormatSupported = false;
        ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, 0));
        Assert(formatCount <= ArrayCount(availableFormats));
        for (u32 i = 0; i < formatCount; ++i)
            ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, &availableFormats[i]));
        for (u32 i = 0; i < formatCount; ++i)
        {
            if (vulkan.surfaceFormat.format == availableFormats[i].format &&
                vulkan.surfaceFormat.colorSpace == availableFormats[i].colorSpace)
                desiredSurfaceFormatSupported = true;
        }
        Assert(desiredSurfaceFormatSupported);
    }

    // NOTE: 2. Get Surface capabilities
    {
        VkSurfaceCapabilitiesKHR surfCapabilities = {};
        ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.gpu, vulkan.surface, &surfCapabilities));

        VkExtent2D swapchainExtent = {};
        if (surfCapabilities.currentExtent.width == UINT32_MAX)
        {
            swapchainExtent.width = width;
            swapchainExtent.height = height;

            if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
                swapchainExtent.width = surfCapabilities.minImageExtent.width;
            else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
                swapchainExtent.width = surfCapabilities.maxImageExtent.width;

            if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
                swapchainExtent.height = surfCapabilities.minImageExtent.height;
            else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
                swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
        else
        { // If the surface size is defined, the swap chain size must match
            swapchainExtent = surfCapabilities.currentExtent;
            Assert(swapchainExtent.width == width);
            Assert(swapchainExtent.height == height);
        }

        // NOTE: 3. Determine the number of VkImage's to use in the swap chain.
        //Constant at 2 for now: Double buffering
        Assert(NUM_SWAPCHAIN_IMAGES >= surfCapabilities.minImageCount);
        Assert(NUM_SWAPCHAIN_IMAGES <= surfCapabilities.maxImageCount);

        // NOTE: 4. Determine the pre-transform
        VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //do nothing
        if (!(surfCapabilities.supportedTransforms & preTransform))
            preTransform = surfCapabilities.currentTransform;

        // NOTE: 5. Set the present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; //The FIFO present mode is guaranteed by the spec to be supported

        //uint32_t presentModeCount;
        //ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, NULL));
        //VkPresentModeKHR presentModes[16];
        //Assert(presentModeCount <= ArrayCount(presentModes));
        //ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, presentModes));
        //bool desiredPresentModeSupported = false;
        //for (u32 i = 0; i < presentModeCount; i++)
        //{
        //	if (presentMode == presentModes[i])
        //		desiredPresentModeSupported = true;
        //}
        //Assert(desiredPresentModeSupported);

        // NOTE: 6. Find a supported composite alpha mode - one of these is guaranteed to be set
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
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
                compositeAlpha = compositeAlphaFlags[i];
                break;
            }
        }

        // NOTE: 7. Make the actual swapchain
        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = vulkan.surface;
        swapchainInfo.imageExtent = swapchainExtent;
        swapchainInfo.imageFormat = vulkan.surfaceFormat.format;
        swapchainInfo.imageColorSpace = vulkan.surfaceFormat.colorSpace;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO: Properly check if supported
        swapchainInfo.minImageCount = NUM_SWAPCHAIN_IMAGES;
        swapchainInfo.preTransform = preTransform;
        swapchainInfo.compositeAlpha = compositeAlpha;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = true;

        u32 queueFamilyIndices[2] = {graphicsQueueFamilyIndex, presentQueueFamilyIndex};
        if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
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

        ASSERT_VK_SUCCESS(vkCreateSwapchainKHR(vulkan.device, &swapchainInfo, 0, &vulkan.swapchain));
    }

    // NOTE: Create the command buffers
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = vulkan.queueFamilyIndex;
        ASSERT_VK_SUCCESS(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));

        VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
        cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
        cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufferAllocInfo.commandBufferCount = NUM_SWAPCHAIN_IMAGES;

        ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, vulkan.cmdBuffers));
    }

    // NOTE: Create the Image views
    {
        ASSERT_VK_SUCCESS(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, 0));
        Assert(vulkan.swapchainImageCount == ArrayCount(vulkan.swapchainImages));
        ASSERT_VK_SUCCESS(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, vulkan.swapchainImages));

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
            ASSERT_VK_SUCCESS(vkCreateImageView(vulkan.device, &imageViewInfo, 0, &vulkan.imageViews[i]));
        }
    }

    // NOTE: Create a depth buffer
    {
        VkImageCreateInfo imageInfo = {};
        vulkan.depthFormat = VK_FORMAT_D16_UNORM;
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vulkan.gpu, vulkan.depthFormat, &props);
        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        else
        {
            Assert(0);
            return false;
        }
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_D16_UNORM;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = NUM_SAMPLES;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkMemoryRequirements depthMemoryReq = {};
        ASSERT_VK_SUCCESS(vkCreateImage(vulkan.device, &imageInfo, 0, &vulkan.depthImage));
        vkGetImageMemoryRequirements(vulkan.device, vulkan.depthImage, &depthMemoryReq);

        u32 memoryIndex = 0; // no use for now
        Assert(VulkanFindMemoryProperties(vulkan.memoryProperties, depthMemoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryIndex));

        VkMemoryAllocateInfo depthMemAllocInfo = {};
        depthMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        depthMemAllocInfo.allocationSize = depthMemoryReq.size;

        ASSERT_VK_SUCCESS(vkAllocateMemory(vulkan.device, &depthMemAllocInfo, 0, &vulkan.depthMemory));
        ASSERT_VK_SUCCESS(vkBindImageMemory(vulkan.device, vulkan.depthImage, vulkan.depthMemory, 0));

        VkImageViewCreateInfo depthImageViewInfo = {};
        depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthImageViewInfo.image = vulkan.depthImage;
        depthImageViewInfo.format = vulkan.depthFormat;
        depthImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        depthImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        depthImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        depthImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthImageViewInfo.subresourceRange.baseMipLevel = 0;
        depthImageViewInfo.subresourceRange.levelCount = 1;
        depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
        depthImageViewInfo.subresourceRange.layerCount = 1;
        depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ASSERT_VK_SUCCESS(vkCreateImageView(vulkan.device, &depthImageViewInfo, 0, &vulkan.depthImageView));
    }
    return true;
}

static void VulkanDestroy(vulkan_state &vulkan)
{
    vkFreeMemory(vulkan.device, vulkan.depthMemory, 0);
    vkDestroyImageView(vulkan.device, vulkan.depthImageView, 0);
    vkDestroyImage(vulkan.device, vulkan.depthImage, 0);

    for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
        vkDestroyImageView(vulkan.device, vulkan.imageViews[i], 0);
    if (!vulkan.swapchain)
        vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);

    vkDestroyCommandPool(vulkan.device, vulkan.cmdPool, 0);

    if (!vulkan.device)
        vkDeviceWaitIdle(vulkan.device);
    vkDestroyDevice(vulkan.device, 0);

    vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
    if (!vulkan.debugMessenger)
        vkDestroyDebugUtilsMessengerEXT(vulkan.instance, vulkan.debugMessenger, 0);
#endif

    if (!vulkan.instance)
        vkDestroyInstance(vulkan.instance, 0);

    if (vulkan.dll)
        FreeLibrary(vulkan.dll);
    return;
}
/*
static bool VulkanRecordCommandBuffer()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkClearColorValue clearColor = {};
    clearColor.
        {1.0f, 0.8f, 0.4f, 0.0f}};

    VkImageSubresourceRange image_subresource_range = {
        VK_IMAGE_ASPECT_COLOR_BIT, // VkImageAspectFlags                     aspectMask
        0,                         // uint32_t                               baseMipLevel
        1,                         // uint32_t                               levelCount
        0,                         // uint32_t                               baseArrayLayer
        1                          // uint32_t                               layerCount
    };

    for (uint32_t i = 0; i < image_count; ++i)
    {
        VkImageMemoryBarrier barrier_from_present_to_clear = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType                        sType
            nullptr,                                // const void                            *pNext
            VK_ACCESS_MEMORY_READ_BIT,              // VkAccessFlags                          srcAccessMask
            VK_ACCESS_TRANSFER_WRITE_BIT,           // VkAccessFlags                          dstAccessMask
            VK_IMAGE_LAYOUT_UNDEFINED,              // VkImageLayout                          oldLayout
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   // VkImageLayout                          newLayout
            VK_QUEUE_FAMILY_IGNORED,                // uint32_t                               srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                // uint32_t                               dstQueueFamilyIndex
            swap_chain_images[i],                   // VkImage                                image
            image_subresource_range                 // VkImageSubresourceRange                subresourceRange
        };

        VkImageMemoryBarrier barrier_from_clear_to_present = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType                        sType
            nullptr,                                // const void                            *pNext
            VK_ACCESS_TRANSFER_WRITE_BIT,           // VkAccessFlags                          srcAccessMask
            VK_ACCESS_MEMORY_READ_BIT,              // VkAccessFlags                          dstAccessMask
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   // VkImageLayout                          oldLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,        // VkImageLayout                          newLayout
            VK_QUEUE_FAMILY_IGNORED,                // uint32_t                               srcQueueFamilyIndex
            VK_QUEUE_FAMILY_IGNORED,                // uint32_t                               dstQueueFamilyIndex
            swap_chain_images[i],                   // VkImage                                image
            image_subresource_range                 // VkImageSubresourceRange                subresourceRange
        };

        vkBeginCommandBuffer(Vulkan.PresentQueueCmdBuffers[i], &cmd_buffer_begin_info);
        vkCmdPipelineBarrier(Vulkan.PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);

        vkCmdClearColorImage(Vulkan.PresentQueueCmdBuffers[i], swap_chain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range);

        vkCmdPipelineBarrier(Vulkan.PresentQueueCmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);
        if (vkEndCommandBuffer(Vulkan.PresentQueueCmdBuffers[i]) != VK_SUCCESS)
        {
            std::cout << "Could not record command buffers!" << std::endl;
            return false;
        }
    }

    return true;
}*/