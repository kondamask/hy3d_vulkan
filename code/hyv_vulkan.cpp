#include "hyv_vulkan.h"

// TODO: make this cross-platform
static bool Vulkan::Win32LoadDLL(vulkan_state &vulkan)
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
static bool Vulkan::LoadGlobalFunctions()
{
    VulkanLoadGlobalFunc(vkCreateInstance);
    VulkanLoadGlobalFunc(vkEnumerateInstanceLayerProperties);
    VulkanLoadGlobalFunc(vkEnumerateInstanceExtensionProperties);
    return true;
}

#define VulkanLoadInstanceFunc(func)                                    \
    func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(instance, #func); \
    if (!(func))                                                        \
    {                                                                   \
        return false;                                                   \
    }
static bool Vulkan::LoadInstanceFunctions(VkInstance instance)
{
    VulkanLoadInstanceFunc(vkDestroyInstance);

#if VULKAN_VALIDATION_LAYERS_ON
    VulkanLoadInstanceFunc(vkCreateDebugUtilsMessengerEXT);
    VulkanLoadInstanceFunc(vkDestroyDebugUtilsMessengerEXT);
#endif

    VulkanLoadInstanceFunc(vkCreateWin32SurfaceKHR); // NOTE: Windows ONLY
    VulkanLoadInstanceFunc(vkDestroySurfaceKHR);

    VulkanLoadInstanceFunc(vkEnumeratePhysicalDevices);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceQueueFamilyProperties);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceSurfaceSupportKHR);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceFormatProperties);
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceMemoryProperties);

    VulkanLoadInstanceFunc(vkEnumerateDeviceExtensionProperties);
    VulkanLoadInstanceFunc(vkCreateDevice);
    VulkanLoadInstanceFunc(vkGetDeviceProcAddr);

    return true;
}

#define VulkanLoadDeviceFunc(func)                                  \
    func = (VulkanFuncPtr(func))vkGetDeviceProcAddr(device, #func); \
    if (!(func))                                                    \
    {                                                               \
        return false;                                               \
    }
static bool Vulkan::LoadDeviceFunctions(VkDevice device)
{
    VulkanLoadDeviceFunc(vkDeviceWaitIdle);
    VulkanLoadDeviceFunc(vkDestroyDevice);
    VulkanLoadDeviceFunc(vkGetDeviceQueue);

    VulkanLoadDeviceFunc(vkCreateSwapchainKHR);
    VulkanLoadDeviceFunc(vkDestroySwapchainKHR);
    VulkanLoadDeviceFunc(vkGetSwapchainImagesKHR);

    VulkanLoadDeviceFunc(vkCreateSemaphore);
    VulkanLoadDeviceFunc(vkDestroySemaphore);
    VulkanLoadDeviceFunc(vkAcquireNextImageKHR);
    VulkanLoadDeviceFunc(vkQueueSubmit);
    VulkanLoadDeviceFunc(vkQueuePresentKHR);

    VulkanLoadDeviceFunc(vkCreateCommandPool);
    VulkanLoadDeviceFunc(vkDestroyCommandPool);
    VulkanLoadDeviceFunc(vkAllocateCommandBuffers);
    VulkanLoadDeviceFunc(vkFreeCommandBuffers);
    VulkanLoadDeviceFunc(vkBeginCommandBuffer);
    VulkanLoadDeviceFunc(vkEndCommandBuffer);
    VulkanLoadDeviceFunc(vkCmdPipelineBarrier);
    VulkanLoadDeviceFunc(vkCmdClearColorImage);

    VulkanLoadDeviceFunc(vkCreateImage);
    VulkanLoadDeviceFunc(vkDestroyImage);
    VulkanLoadDeviceFunc(vkCreateImageView);
    VulkanLoadDeviceFunc(vkDestroyImageView);
    VulkanLoadDeviceFunc(vkGetImageMemoryRequirements);
    VulkanLoadDeviceFunc(vkBindImageMemory);

    VulkanLoadDeviceFunc(vkAllocateMemory);
    VulkanLoadDeviceFunc(vkFreeMemory);
    return true;
}

static bool Vulkan::FindMemoryProperties(VkPhysicalDeviceMemoryProperties &memoryProperties,
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
static VKAPI_ATTR VkBool32 VKAPI_CALL
Vulkan::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
    DebugPrint(pCallbackData->pMessageIdName);
    DebugPrint('\n');

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
            DebugPrint(c);
            i++;
        }
        else
        {
            DebugPrint('\n');
            i++;
            do
            {
                c = pCallbackData->pMessage[i];
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

static bool Vulkan::CreateCommandBuffers(vulkan_state &vulkan)
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = vulkan.presentQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_VK_SUCCESS(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));

    VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = vulkan.swapchainImageCount;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, vulkan.cmdBuffers));

    return true;
}

static bool Vulkan::CreateSwapchain(vulkan_state &vulkan)
{
    if (VulkanValidHandle(vulkan.device))
        vkDeviceWaitIdle(vulkan.device);
    else
        return false;

    vulkan.canRender = false;

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

        VkExtent2D desiredExtent = {};
        if (surfCapabilities.currentExtent.width == UINT32_MAX)
        {
            desiredExtent.width = WINDOW_WIDTH;
            desiredExtent.height = WINDOW_HEIGHT;

            if (desiredExtent.width < surfCapabilities.minImageExtent.width)
                desiredExtent.width = surfCapabilities.minImageExtent.width;
            else if (desiredExtent.width > surfCapabilities.maxImageExtent.width)
                desiredExtent.width = surfCapabilities.maxImageExtent.width;

            if (desiredExtent.height < surfCapabilities.minImageExtent.height)
                desiredExtent.height = surfCapabilities.minImageExtent.height;
            else if (desiredExtent.height > surfCapabilities.maxImageExtent.height)
                desiredExtent.height = surfCapabilities.maxImageExtent.height;
        }
        else
        { // If the surface size is defined, the swap chain size must match
            desiredExtent = surfCapabilities.currentExtent;
        }
        if ((desiredExtent.width == 0) || (desiredExtent.height == 0))
        {
            // Current surface size is (0, 0) so we can't create a swap chain and render anything (CanRender == false)
            // But we don't wont to kill the application as this situation may occur i.e. when window gets minimized
            return true;
        }

        // NOTE: 3. Determine the number of VkImage's to use in the swap chain.
        //Constant at 2 for now: Double buffering
        Assert(NUM_SWAPCHAIN_IMAGES >= surfCapabilities.minImageCount);
        Assert(NUM_SWAPCHAIN_IMAGES <= surfCapabilities.maxImageCount);
        vulkan.swapchainImageCount = NUM_SWAPCHAIN_IMAGES;

        // NOTE: 4. Determine the pre-transform
        VkSurfaceTransformFlagBitsKHR desiredPreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //do nothing
        if (!(surfCapabilities.supportedTransforms & desiredPreTransform))
            desiredPreTransform = surfCapabilities.currentTransform;

        // NOTE: 5. Set the present mode
        VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR; //The FIFO present mode is guaranteed by the spec to be supported

        //uint32_t presentModeCount;
        //ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, NULL));
        //VkPresentModeKHR presentModes[16];
        //Assert(presentModeCount <= ArrayCount(presentModes));
        //ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan.gpu, vulkan.surface, &presentModeCount, presentModes));
        //bool desiredPresentModeSupported = false;
        //for (u32 i = 0; i < presentModeCount; i++)
        //{
        //	if (desiredPresentMode == presentModes[i])
        //		desiredPresentModeSupported = true;
        //}
        //Assert(desiredPresentModeSupported);

        // NOTE: 6. Find a supported composite alpha mode - one of these is guaranteed to be set
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

        // NOTE: 7. Set image usage flags
        VkImageUsageFlags desiredImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //must alwasy be supported
        if (surfCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            desiredImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        else
            DebugPrint("VK_IMAGE_USAGE_TRANSFER_DST_BIT not supported\n");

        // NOTE: 8. Make the actual swapchain
        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = vulkan.surface;
        swapchainInfo.imageExtent = desiredExtent;
        swapchainInfo.imageFormat = vulkan.surfaceFormat.format;
        swapchainInfo.imageColorSpace = vulkan.surfaceFormat.colorSpace;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = desiredImageUsageFlags;
        swapchainInfo.minImageCount = vulkan.swapchainImageCount;
        swapchainInfo.preTransform = desiredPreTransform;
        swapchainInfo.compositeAlpha = desiredCompositeAlpha;
        swapchainInfo.presentMode = desiredPresentMode;
        swapchainInfo.clipped = true;

        u32 queueFamilyIndices[2] = {vulkan.graphicsQueueFamilyIndex, vulkan.presentQueueFamilyIndex};
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
        ASSERT_VK_SUCCESS(vkCreateSwapchainKHR(vulkan.device, &swapchainInfo, 0, &vulkan.swapchain));
        if (VulkanValidHandle(oldSwapchain)) //old swapchain
            vkDestroySwapchainKHR(vulkan.device, oldSwapchain, 0);
    }

    // NOTE: Create the Image views
    {
        Vulkan::ClearSwapchainImages(vulkan);
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
            ASSERT_VK_SUCCESS(vkCreateImageView(vulkan.device, &imageViewInfo, 0, &vulkan.swapchainImageViews[i]));
        }
    }
    vulkan.canRender = true;

    return true;
}

// TODO: Make it cross-platform
static bool Vulkan::Win32Initialize(vulkan_state &vulkan, HINSTANCE &wndInstance, HWND &wndHandle, const char *name)
{
    if (!Vulkan::LoadGlobalFunctions())
    {
        return false;
    }

#if VULKAN_VALIDATION_LAYERS_ON
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
    debugMessengerInfo.pfnUserCallback = Vulkan::DebugCallback;

    char *desiredInstanceExtensions[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                          VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                                          VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
                                          VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
                                          VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
    };
#else
    char *desiredInstanceExtensions[] = { VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                                          VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
                                          VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
                                          VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
    };
#endif

    // NOTE: Create an instance
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = name;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        u32 instanceExtensionsCount;
        VkExtensionProperties availableInstanceExtensions[255] = {};
        ASSERT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(0, &instanceExtensionsCount, 0));
        Assert(instanceExtensionsCount <= ArrayCount(availableInstanceExtensions));
        ASSERT_VK_SUCCESS(vkEnumerateInstanceExtensionProperties(0, &instanceExtensionsCount, availableInstanceExtensions));
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
#if VULKAN_VALIDATION_LAYERS_ON
        instanceInfo.enabledLayerCount = ArrayCount(validationLayers);
        instanceInfo.ppEnabledLayerNames = validationLayers;
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif

        ASSERT_VK_SUCCESS(vkCreateInstance(&instanceInfo, NULL, &vulkan.instance));
        if (!Vulkan::LoadInstanceFunctions(vulkan.instance))
        {
            return false;
        }

#if VULKAN_VALIDATION_LAYERS_ON
        ASSERT_VK_SUCCESS(vkCreateDebugUtilsMessengerEXT(vulkan.instance, &debugMessengerInfo, 0, &vulkan.debugMessenger));
#endif
    }

    // NOTE: Create a surface
    {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.hinstance = wndInstance;
        surfaceInfo.hwnd = wndHandle;
        ASSERT_VK_SUCCESS(vkCreateWin32SurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));

#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.connection = 0; //we'll have these if we implement a linux window
        surfaceInfo.window = 0;     //we'll have these if we implement a linux window
        ASSERT_VK_SUCCESS(vkCreateXcbSurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.dpy = 0;    //we'll have these if we implement a mac(?) window
        surfaceInfo.window = 0; //we'll have these if we implement a mac(?) window
        ASSERT_VK_SUCCESS(vkCreateXlibSurfaceKHR(vulkan.instance, &surfaceInfo, 0, &vulkan.surface));
#endif
    }

    // NOTE: Select a gpu to use
    {
        u32 gpuCount = 0;
        VkPhysicalDevice gpuBuffer[16] = {};
        ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, 0));
        Assert(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
        ASSERT_VK_SUCCESS(vkEnumeratePhysicalDevices(vulkan.instance, &gpuCount, gpuBuffer));
        vulkan.gpu = gpuBuffer[0];
        // TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
        //For now it's ok since I only have 1 gpu.

        vkGetPhysicalDeviceMemoryProperties(vulkan.gpu, &vulkan.memoryProperties);
    }

    // NOTE: Pick a queue family the supports both present and graphics operations
    vulkan.graphicsQueueFamilyIndex = UINT32_MAX;
    vulkan.presentQueueFamilyIndex = UINT32_MAX;
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
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        u32 deviceExtensionsCount;
        VkExtensionProperties availableDeviceExtensions[255] = {};
        ASSERT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, 0));
        Assert(deviceExtensionsCount <= ArrayCount(availableDeviceExtensions));
        ASSERT_VK_SUCCESS(vkEnumerateDeviceExtensionProperties(vulkan.gpu, 0, &deviceExtensionsCount, availableDeviceExtensions));
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

        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = ArrayCount(desiredDeviceExtensions);
        deviceInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
        ASSERT_VK_SUCCESS(vkCreateDevice(vulkan.gpu, &deviceInfo, 0, &vulkan.device));
        if (!Vulkan::LoadDeviceFunctions(vulkan.device))
        {
            Assert("ERROR: Device Functions not loaded\n");
            return false;
        }
    }

    // NOTE: Get the queues and save it into our vulkan object
    vkGetDeviceQueue(vulkan.device, vulkan.graphicsQueueFamilyIndex, 0, &vulkan.graphicsQueue);
    vkGetDeviceQueue(vulkan.device, vulkan.presentQueueFamilyIndex, 0, &vulkan.presentQueue);

    // NOTE: Create semaphores
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        ASSERT_VK_SUCCESS(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.imageAvailableSem));
        ASSERT_VK_SUCCESS(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.renderFinishedSem));
    }

    // NOTE: Create a depth buffer
    // TODO: extract it in a function and update it OnWindowSizeChange
    /*{
        VkImageCreateInfo depthImageInfo = {};
        vulkan.depthFormat = VK_FORMAT_D16_UNORM;
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vulkan.gpu, vulkan.depthFormat, &props);
        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            depthImageInfo.tiling = VK_IMAGE_TILING_LINEAR;
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        else
        {
            Assert(0);
            return false;
        }
        depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageInfo.format = VK_FORMAT_D16_UNORM;
        depthImageInfo.extent.width = WINDOW_WIDTH;
        depthImageInfo.extent.height = WINDOW_HEIGHT;
        depthImageInfo.extent.depth = 1;
        depthImageInfo.mipLevels = 1;
        depthImageInfo.arrayLayers = 1;
        depthImageInfo.samples = NUM_SAMPLES;
        depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkMemoryRequirements depthMemoryReq = {};
        ASSERT_VK_SUCCESS(vkCreateImage(vulkan.device, &depthImageInfo, 0, &vulkan.depthImage));
        vkGetImageMemoryRequirements(vulkan.device, vulkan.depthImage, &depthMemoryReq);

        u32 memoryIndex = 0; // no use for now
        if (!Vulkan::FindMemoryProperties(vulkan.memoryProperties, depthMemoryReq.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryIndex))
            return false;

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
    }*/

    if (!Vulkan::CreateSwapchain(vulkan))
    {
        return false;
    }
    if (!Vulkan::CreateCommandBuffers(vulkan))
    {
        return false;
    }

    vulkan.canRender = true;

    return true;
}

static void Vulkan::ClearCommands(vulkan_state &vulkan)
{
    if (VulkanValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);

        if (VulkanValidHandle(vulkan.cmdBuffers[0]) && VulkanValidHandle(vulkan.cmdBuffers[1])) // TODO: For now we only support double buffering
        {
            vkFreeCommandBuffers(vulkan.device, vulkan.cmdPool, ArrayCount(vulkan.cmdBuffers), vulkan.cmdBuffers);
            vulkan.cmdBuffers[0] = 0;
            vulkan.cmdBuffers[1] = 0;
        }
        if (VulkanValidHandle(vulkan.cmdPool))
        {
            vkDestroyCommandPool(vulkan.device, vulkan.cmdPool, 0);
            vulkan.cmdPool = 0;
        }
    }
}

static void Vulkan::ClearSwapchainImages(vulkan_state &vulkan)
{
    if (VulkanValidHandle(vulkan.device))
    {
        for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
        {
            if (VulkanValidHandle(vulkan.swapchainImageViews[i]))
                vkDestroyImageView(vulkan.device, vulkan.swapchainImageViews[i], 0);
        }
    }
}

static void Vulkan::Destroy(vulkan_state &vulkan)
{
    if (VulkanValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);

        if (VulkanValidHandle(vulkan.depthMemory))
            vkFreeMemory(vulkan.device, vulkan.depthMemory, 0);

        if (VulkanValidHandle(vulkan.depthImageView))
            vkDestroyImageView(vulkan.device, vulkan.depthImageView, 0);

        if (VulkanValidHandle(vulkan.depthImage))
            vkDestroyImage(vulkan.device, vulkan.depthImage, 0);

        Vulkan::ClearSwapchainImages(vulkan);
        if (VulkanValidHandle(vulkan.swapchain))
            vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);

        if (VulkanValidHandle(vulkan.imageAvailableSem))
            vkDestroySemaphore(vulkan.device, vulkan.imageAvailableSem, 0);
        if (VulkanValidHandle(vulkan.renderFinishedSem))
            vkDestroySemaphore(vulkan.device, vulkan.renderFinishedSem, 0);

        Vulkan::ClearCommands(vulkan);

        vkDestroyDevice(vulkan.device, 0);
    }

    if (VulkanValidHandle(vulkan.instance))
    {
        if (VulkanValidHandle(vulkan.surface))
            vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
        if (VulkanValidHandle(vulkan.debugMessenger))
            vkDestroyDebugUtilsMessengerEXT(vulkan.instance, vulkan.debugMessenger, 0);
#endif

        vkDestroyInstance(vulkan.instance, 0);
    }

    if (vulkan.dll)
        FreeLibrary(vulkan.dll);
    return;
}

static bool Vulkan::ClearScreen(vulkan_state &vulkan, float r)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkClearColorValue clearColor = VulkanClearColor(r, 0.8f, 0.4f, 0.0f);

    VkImageSubresourceRange imageSubresourceRange = {};
    imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = 1;
    imageSubresourceRange.baseArrayLayer = 0;
    imageSubresourceRange.layerCount = 1;

    for (u32 i = 0; i < vulkan.swapchainImageCount; ++i)
    {
        VkImageMemoryBarrier barrierFromPresentToClear = {};
        barrierFromPresentToClear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierFromPresentToClear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrierFromPresentToClear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierFromPresentToClear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrierFromPresentToClear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierFromPresentToClear.srcQueueFamilyIndex = vulkan.presentQueueFamilyIndex;
        barrierFromPresentToClear.dstQueueFamilyIndex = vulkan.presentQueueFamilyIndex;
        barrierFromPresentToClear.image = vulkan.swapchainImages[i];
        barrierFromPresentToClear.subresourceRange = imageSubresourceRange;

        VkImageMemoryBarrier barrierFromClearToPresent = {};
        barrierFromClearToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierFromClearToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierFromClearToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrierFromClearToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierFromClearToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrierFromClearToPresent.srcQueueFamilyIndex = vulkan.presentQueueFamilyIndex;
        barrierFromClearToPresent.dstQueueFamilyIndex = vulkan.presentQueueFamilyIndex;
        barrierFromClearToPresent.image = vulkan.swapchainImages[i];
        barrierFromClearToPresent.subresourceRange = imageSubresourceRange;

        ASSERT_VK_SUCCESS(vkBeginCommandBuffer(vulkan.cmdBuffers[i], &commandBufferBeginInfo));
        vkCmdPipelineBarrier(vulkan.cmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, 0, 0, 0, 1, &barrierFromPresentToClear);
        vkCmdClearColorImage(vulkan.cmdBuffers[i], vulkan.swapchainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             &clearColor, 1, &imageSubresourceRange);
        vkCmdPipelineBarrier(vulkan.cmdBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, 0, 0, 0, 1, &barrierFromClearToPresent);
        ASSERT_VK_SUCCESS(vkEndCommandBuffer(vulkan.cmdBuffers[i]));
    }
    return true;
}

static bool Vulkan::OnWindowSizeChange(vulkan_state &vulkan)
{
    Vulkan::ClearCommands(vulkan);
    if (!Vulkan::CreateSwapchain(vulkan))
        return false;
    if (!Vulkan::CreateCommandBuffers(vulkan))
        return false;
    return true;
}

static bool Vulkan::Draw(vulkan_state &vulkan)
{
    // NOTE: Get the image
    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX,
                                            vulkan.imageAvailableSem, 0, &imageIndex);
    switch (result)
    {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        return Vulkan::OnWindowSizeChange(vulkan);
    default:
        DebugPrint("Problem occurred during swap chain image acquisition!\n");
        return false;
    }

    // NOTE: Wait on imageAvailableSem and submit the command buffer and signal renderFinishedSem
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkan.imageAvailableSem;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan.cmdBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkan.renderFinishedSem;
    vkQueueSubmit(vulkan.presentQueue, 1, &submitInfo, 0);

    // NOTE: Submit image to present when signaled by renderFinishedSem
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vulkan.renderFinishedSem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkan.swapchain;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(vulkan.presentQueue, &presentInfo);

    switch (result)
    {
    case VK_SUCCESS:
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        return Vulkan::OnWindowSizeChange(vulkan);
    default:
        DebugPrint("Problem occurred during image presentation!\n");
        return false;
    }
    return true;
}

bool Vulkan::CanRender(vulkan_state &vulkan)
{
    return vulkan.canRender;
}
