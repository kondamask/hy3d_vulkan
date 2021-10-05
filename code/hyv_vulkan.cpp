#include "hyv_vulkan.h"

// TODO: make this cross-platform
static bool Win32LoadVulkan()
{
    HMODULE vulkanDLL = LoadLibraryA("vulkan-1.dll");
    if (!vulkanDLL)
        return false;

    vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkanDLL, "vkGetInstanceProcAddr");
    return true;
}

#define VK_LOAD_GLOBAL_FUNCTION(func)                       \
    func = (PFN_##func)vkGetInstanceProcAddr(nullptr, #func); \
    if (!(func))                                            \
    {                                                      \
        return false;                                      \
    }
static bool VkLoadGlobalFunctions()
{
    VK_LOAD_GLOBAL_FUNCTION(vkCreateInstance)
    VK_LOAD_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties)
    return true;
}

#define VK_LOAD_INSTANCE_FUNCTION(instance, func)            \
    func = (PFN_##func)vkGetInstanceProcAddr(instance, #func); \
    if (!(func))                                             \
    {                                                       \
        return false;                                       \
    }
static bool VkLoadInstanceFunctions(VkInstance instance)
{
    VK_LOAD_INSTANCE_FUNCTION(instance, vkCreateWin32SurfaceKHR)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkDestroySurfaceKHR)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkEnumeratePhysicalDevices)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceQueueFamilyProperties)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceSupportKHR)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceFormatsKHR)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetPhysicalDeviceFormatProperties)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkCreateDevice)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkGetDeviceProcAddr)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkCreateDebugUtilsMessengerEXT)
    VK_LOAD_INSTANCE_FUNCTION(instance, vkDestroyInstance)
    return true;
}

#define VK_LOAD_DEVICE_FUNCTION(device, func)            \
    func = (PFN_##func)vkGetDeviceProcAddr(device, #func); \
    if (!(func))                                         \
    {                                                   \
        return false;                                   \
    }
static bool VkLoadDeviceFunctions(VkDevice device)
{
    VK_LOAD_DEVICE_FUNCTION(device, vkGetDeviceQueue)
    VK_LOAD_DEVICE_FUNCTION(device, vkCreateCommandPool)
    VK_LOAD_DEVICE_FUNCTION(device, vkAllocateCommandBuffers)
    VK_LOAD_DEVICE_FUNCTION(device, vkCreateSwapchainKHR)
    VK_LOAD_DEVICE_FUNCTION(device, vkGetSwapchainImagesKHR)
    VK_LOAD_DEVICE_FUNCTION(device, vkCreateImageView)
    VK_LOAD_DEVICE_FUNCTION(device, vkCreateImage)
    VK_LOAD_DEVICE_FUNCTION(device, vkDestroyImage)
    VK_LOAD_DEVICE_FUNCTION(device, vkDestroyImageView)
    VK_LOAD_DEVICE_FUNCTION(device, vkDestroySwapchainKHR)
    VK_LOAD_DEVICE_FUNCTION(device, vkDestroyCommandPool)
    VK_LOAD_DEVICE_FUNCTION(device, vkDeviceWaitIdle)
    VK_LOAD_DEVICE_FUNCTION(device, vkDestroyDevice)
    return true;
}

#if VULKAN_VALIDATION_LAYERS_ON
#include <iostream>
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
        ASSERT(0);
        return VK_FALSE;
    }
    return VK_FALSE;
}
#endif

static bool Win32InitializeVulkan(vulkan_state &vulkan, HINSTANCE &wndInstance, HWND &wndHandle, const char *name, u32 width, u32 height)
{
    ASSERT(VkLoadGlobalFunctions());

#if VULKAN_VALIDATION_LAYERS_ON
    // NOTE: Enable Validation Layer
    const bool enableValidationLayers = true;

    char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"};

    u32 layerCount;
    VK_FUNC_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, 0));

    VkLayerProperties availableLayers[16];
    ASSERT(layerCount <= ArrayCount(availableLayers));
    VK_FUNC_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        ASSERT(layerFound);
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

    VK_FUNC_ASSERT(vkCreateInstance(&instanceInfo, NULL, &vulkan.instance));
    ASSERT(VkLoadInstanceFunctions(vulkan.instance));

#if VULKAN_VALIDATION_LAYERS_ON
    VK_FUNC_ASSERT(vkCreateDebugUtilsMessengerEXT(vulkan.instance, &debugMessengerInfo, 0, &vulkan.debugMessenger));
#endif

    // NOTE: Create a surface
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.hinstance = wndInstance;
        surfaceInfo.hwnd = wndHandle;
        VK_FUNC_ASSERT(vkCreateWin32SurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));
    }

    // NOTE: Select a gpu to use
    {
        u32 gpuCount = 0;
        VK_FUNC_ASSERT(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, 0));

        VkPhysicalDevice gpuBuffer[16] = {};
        ASSERT(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
        VK_FUNC_ASSERT(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, gpuBuffer));
        vulkan.gpu = gpuBuffer[0];
        // TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
        //For now it's ok since I only have 1 gpu.
    }

    // NOTE: Pick a queue family the supports both present and graphics operations
    u32 graphicsQueueFamilyIndex = UINT32_MAX;
    u32 presentQueueFamilyIndex = UINT32_MAX;
    {
        u32 queueFamilyCount;
        VkQueueFamilyProperties availableQueueFamilies[16] = {};
        VkBool32 supportsPresent[16] = {};
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, 0);
        ASSERT(queueFamilyCount <= ArrayCount(availableQueueFamilies));
        vkGetPhysicalDeviceQueueFamilyProperties(vulkan.gpu, &queueFamilyCount, availableQueueFamilies);

        for (u32 i = 0; i < queueFamilyCount; i++)
            VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(vulkan.gpu, i, vulkan.surface, &supportsPresent[i]));

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
            ASSERT("ERROR: No graphics queue found.\n");
            return false;
        }
        if (presentQueueFamilyIndex == UINT32_MAX)
        {
            ASSERT("ERROR: No present queue found.\n");
            return false;
        }
    }

    ASSERT(graphicsQueueFamilyIndex == presentQueueFamilyIndex);
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

        char *deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        // TODO: MUST check if swapchain is supported as an extention for this device
        //with vkEnumerateDeviceExtensionProperties

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = ArrayCount(deviceExtensions);
        deviceInfo.ppEnabledExtensionNames = deviceExtensions;
        VK_FUNC_ASSERT(vkCreateDevice(vulkan.gpu, &deviceInfo, 0, &vulkan.device));
    }

    ASSERT(VkLoadDeviceFunctions(vulkan.device));

    // NOTE: Get the queue and save it into our vulkan object
    vkGetDeviceQueue(vulkan.device, queueInfo.queueFamilyIndex, 0, &vulkan.queue);

    // NOTE: Create a command buffer
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueInfo.queueFamilyIndex;
        VK_FUNC_ASSERT(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));

        VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
        cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
        cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufferAllocInfo.commandBufferCount = 1;

        VK_FUNC_ASSERT(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, &vulkan.cmdBuffer));
    }

    // NOTE: Create a swapchain

    // NOTE: 1.set a proper surface format
    {
        vulkan.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        vulkan.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        u32 formatCount = 0;
        VkSurfaceFormatKHR availableFormats[16] = {};
        bool desiredSurfaceFormatSupported = false;
        VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, 0));
        ASSERT(formatCount <= ArrayCount(availableFormats));
        for (u32 i = 0; i < formatCount; ++i)
            VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan.gpu, vulkan.surface, &formatCount, &availableFormats[i]));
        for (u32 i = 0; i < formatCount; ++i)
        {
            if (vulkan.surfaceFormat.format == availableFormats[i].format &&
                vulkan.surfaceFormat.colorSpace == availableFormats[i].colorSpace)
                desiredSurfaceFormatSupported = true;
        }
        ASSERT(desiredSurfaceFormatSupported);
    }

    // NOTE: 2. Get Surface capabilities
    {
        VkSurfaceCapabilitiesKHR surfCapabilities = {};
        VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.gpu, vulkan.surface, &surfCapabilities));

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
            ASSERT(swapchainExtent.width == width);
            ASSERT(swapchainExtent.height == height);
        }

        // NOTE: 3. Determine the number of VkImage's to use in the swap chain.
        //Constant at 2 for now: Double buffering
        ASSERT(NUM_SWAPCHAIN_IMAGES <= surfCapabilities.maxImageCount);

        // NOTE: 4. Determine the pre-transform
        VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //do nothing
        if (!(surfCapabilities.supportedTransforms & preTransform))
            preTransform = surfCapabilities.currentTransform;

        // NOTE: 5. Set the present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; //The FIFO present mode is guaranteed by the spec to be supported

        //uint32_t presentModeCount;
        //VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, NULL));
        //VkPresentModeKHR presentModes[16];
        //ASSERT(presentModeCount <= ArrayCount(presentModes));
        //VK_FUNC_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, presentModes));
        //bool desiredPresentModeSupported = false;
        //for (u32 i = 0; i < presentModeCount; i++)
        //{
        //	if (presentMode == presentModes[i])
        //		desiredPresentModeSupported = true;
        //}
        //ASSERT(desiredPresentModeSupported);

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
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

        VK_FUNC_ASSERT(vkCreateSwapchainKHR(vulkan.device, &swapchainInfo, 0, &vulkan.swapchain));
    }

    // NOTE: Create the Image views
    {
        VK_FUNC_ASSERT(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, 0));
        ASSERT(vulkan.swapchainImageCount == ArrayCount(vulkan.swapchainImages));
        VK_FUNC_ASSERT(vkGetSwapchainImagesKHR(vulkan.device, vulkan.swapchain, &vulkan.swapchainImageCount, vulkan.swapchainImages));

        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = vulkan.surfaceFormat.format;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; //VK_COMPONENT_SWIZZLE_G;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; //VK_COMPONENT_SWIZZLE_B;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; //VK_COMPONENT_SWIZZLE_A;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
        {
            imageViewInfo.image = vulkan.swapchainImages[i];
            VK_FUNC_ASSERT(vkCreateImageView(vulkan.device, &imageViewInfo, 0, &vulkan.imageViews[i]));
        }
    }

    // NOTE: Create a depth buffer
    VkImageCreateInfo imageInfo = {};
    VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(vulkan.gpu, depthFormat, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    else
    {
        ASSERT(0);
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

    vkCreateImage(vulkan.device, &imageInfo, 0, &vulkan.depthImage);

    return true;
}

static void Win32DestroyVulkan(vulkan_state &vulkan)
{
    vkDestroyImage(vulkan.device, vulkan.depthImage, 0);
    for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
    {
        vkDestroyImageView(vulkan.device, vulkan.imageViews[i], 0);
    }
    vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);
    vkDestroyCommandPool(vulkan.device, vulkan.cmdPool, 0);
    vkDeviceWaitIdle(vulkan.device);
    vkDestroyDevice(vulkan.device, 0);
    vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
    PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (destroyDebugMessenger != nullptr)
    {
        destroyDebugMessenger(vulkan.instance, vulkan.debugMessenger, 0);
    }
#endif

    vkDestroyInstance(vulkan.instance, 0);
    return;
}
