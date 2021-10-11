#include "hy3d_vulkan.h"

#if VULKAN_VALIDATION_LAYERS_ON
function VKAPI_ATTR VkBool32 VKAPI_CALL
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

#define VulkanLoadGlobalFunc(func)                                     \
func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(nullptr, #func); \
if (!(func))                                                       \
{                                                                  \
return false;                                                  \
}
function bool Vulkan::LoadGlobalFunctions()
{
    //test
    VulkanLoadGlobalFunc(vkCreateInstance);
    VulkanLoadGlobalFunc(vkEnumerateInstanceLayerProperties);
    VulkanLoadGlobalFunc(vkEnumerateInstanceExtensionProperties);
    
    DebugPrint("Loaded Global Functions\n");
    
    return true;
}

#define VulkanLoadInstanceFunc(func)                                           \
func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(vulkan.instance, #func); \
if (!(func))                                                               \
{                                                                          \
return false;                                                          \
}
function bool Vulkan::LoadInstanceFunctions()
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
    
    DebugPrint("Loaded Instance Functions\n");
    
    return true;
}

#define VulkanLoadDeviceFunc(func)                                         \
func = (VulkanFuncPtr(func))vkGetDeviceProcAddr(vulkan.device, #func); \
if (!(func))                                                           \
{                                                                      \
return false;                                                      \
}
function bool Vulkan::LoadDeviceFunctions()
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
    VulkanLoadDeviceFunc(vkResetCommandPool);
    VulkanLoadDeviceFunc(vkDestroyCommandPool);
    VulkanLoadDeviceFunc(vkAllocateCommandBuffers);
    VulkanLoadDeviceFunc(vkFreeCommandBuffers);
    VulkanLoadDeviceFunc(vkBeginCommandBuffer);
    VulkanLoadDeviceFunc(vkEndCommandBuffer);
    VulkanLoadDeviceFunc(vkResetCommandBuffer);
    VulkanLoadDeviceFunc(vkCreateImage);
    VulkanLoadDeviceFunc(vkDestroyImage);
    VulkanLoadDeviceFunc(vkCreateImageView);
    VulkanLoadDeviceFunc(vkDestroyImageView);
    VulkanLoadDeviceFunc(vkGetImageMemoryRequirements);
    VulkanLoadDeviceFunc(vkBindImageMemory);
    VulkanLoadDeviceFunc(vkAllocateMemory);
    VulkanLoadDeviceFunc(vkFreeMemory);
    VulkanLoadDeviceFunc(vkCreateRenderPass);
    VulkanLoadDeviceFunc(vkDestroyRenderPass);
    VulkanLoadDeviceFunc(vkCreateFramebuffer);
    VulkanLoadDeviceFunc(vkDestroyFramebuffer);
    VulkanLoadDeviceFunc(vkCreateFence);
    VulkanLoadDeviceFunc(vkDestroyFence);
    VulkanLoadDeviceFunc(vkWaitForFences);
    VulkanLoadDeviceFunc(vkResetFences);
    VulkanLoadDeviceFunc(vkCreateShaderModule);
    VulkanLoadDeviceFunc(vkDestroyShaderModule);
    VulkanLoadDeviceFunc(vkCreatePipelineLayout);
    VulkanLoadDeviceFunc(vkDestroyPipelineLayout);
    VulkanLoadDeviceFunc(vkCreateGraphicsPipelines);
    VulkanLoadDeviceFunc(vkDestroyPipeline);
    
    VulkanLoadDeviceFunc(vkCmdBeginRenderPass);
    VulkanLoadDeviceFunc(vkCmdEndRenderPass);
    VulkanLoadDeviceFunc(vkCmdPipelineBarrier);
    VulkanLoadDeviceFunc(vkCmdClearColorImage);
    VulkanLoadDeviceFunc(vkCmdBindPipeline);
    VulkanLoadDeviceFunc(vkCmdDraw);
    
    DebugPrint("Loaded Device Functions\n");
    
    return true;
}

function bool Vulkan::FindMemoryProperties(VkPhysicalDeviceMemoryProperties &memoryProperties,
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

// TODO: make this cross-platform
function bool Vulkan::Win32LoadDLL()
{
    vulkan.dll = LoadLibraryA("vulkan-1.dll");
    if (!vulkan.dll)
        return false;
    
    vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkan.dll, "vkGetInstanceProcAddr");
    
    DebugPrint("Loaded DLL\n");
    return true;
}


// TODO: Make it cross-platform
function bool Vulkan::Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name)
{
    DebugPrint("Initialize Vulkan\n");
    if (!Win32LoadDLL())
    {
        return false;
    }
    
    if (!LoadGlobalFunctions())
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
    debugMessengerInfo.pfnUserCallback = DebugCallback;
    
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
    
    DebugPrint("Created Vulkan Debug Messenger\n");
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
        DebugPrint("Created Vulkan Instance\n");
        
        if (!LoadInstanceFunctions())
        {
            Assert("ERROR: Instance Functions not loaded\n");
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
        
        DebugPrint("Selected a GPU\n");
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
        DebugPrint("\nCreated a Vulkan Device\n");
        
        if (!LoadDeviceFunctions())
        {
            Assert("ERROR: Device Functions not loaded\n");
            return false;
        }
    }
    
    // NOTE: Get the queues and save it into our vulkan object
    vkGetDeviceQueue(vulkan.device, vulkan.graphicsQueueFamilyIndex, 0, &vulkan.graphicsQueue);
    vkGetDeviceQueue(vulkan.device, vulkan.presentQueueFamilyIndex, 0, &vulkan.presentQueue);
    
    // NOTE: Create semaphores and fence
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        ASSERT_VK_SUCCESS(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.imageAvailableSem));
        ASSERT_VK_SUCCESS(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.renderFinishedSem));
        
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        ASSERT_VK_SUCCESS(vkCreateFence(vulkan.device, &fenceInfo, 0, &vulkan.renderFence));
        
        DebugPrint("Created Semaphores and Fence\n");
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
        if (!FindMemoryProperties(vulkan.memoryProperties, depthMemoryReq.memoryTypeBits,
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
    
    // NOTE: Set a proper surface format
    {
        vulkan.surfaceFormat.colorSpace = SURFACE_FORMAT_COLOR_SPACE;
        vulkan.surfaceFormat.format = SURFACE_FORMAT_FORMAT;
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
    
    // NOTE: Create a renderpass
    {
        //The following structures are, in general, parallel arrays that describe a subpass.
        //Here we only have one subpass for now.
        
        //We need to create an attachement
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.flags = 0;
        colorAttachment.format = vulkan.surfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference colorAttachementRef = {};
        colorAttachementRef.attachment = 0;
        colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpassDescriptions = {};
        //subpassDescriptions.flags = 0;
        subpassDescriptions.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        
        // NOTE(heyyod): The index of the attachment in this array is directly 
        //referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
        subpassDescriptions.colorAttachmentCount = 1; // ArrayCount(colorAttachementRef)
        subpassDescriptions.pColorAttachments = &colorAttachementRef;
        
        //subpassDescriptions.inputAttachmentCount = 0;
        //subpassDescriptions.pInputAttachments = 0;
        //subpassDescriptions.pResolveAttachments = 0;
        //subpassDescriptions.pDepthStencilAttachment = 0;
        //subpassDescriptions.preserveAttachmentCount = 0;
        //subpassDescriptions.pPreserveAttachments = 0;
        
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescriptions;
        ASSERT_VK_SUCCESS(vkCreateRenderPass(vulkan.device, &renderPassInfo, 0, &vulkan.renderPass));
        
        DebugPrint("Created a renderpass\n");
    }
    
    // NOTE(heyyod): It looks like window always sends a WM_SIZE message
    // at startup. This Recreats the following. So we don't need to create these here
    /* 
        if (!CreateSwapchain())
        {
            Assert("Failed to create swapchain");
            return false;
        }
        
        if (!CreateCommandBuffers())
        {
            Assert("Failed to create command buffers");
            return false;
        }
        
        if (!CreateFrameBuffers())
        {
            Assert("Failed to create framebuffers");
            return false;
        }
        
        if(!CreatePipeline())
        {
            Assert("Could not create pipeline");
            return false;
        }
         */
    
    vulkan.canRender = true;
    return true;
}

function bool Vulkan::CreateFrameBuffers()
{
    ClearFrameBuffers();
    
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkan.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = vulkan.windowExtent.width;
    framebufferInfo.height = vulkan.windowExtent.height;
    framebufferInfo.layers = 1;
    for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
    {
        framebufferInfo.pAttachments = &vulkan.swapchainImageViews[i];
        ASSERT_VK_SUCCESS(vkCreateFramebuffer(vulkan.device, &framebufferInfo, 0, &vulkan.framebuffers[i]));
    }
    DebugPrint("Created FrameBuffers\n");
    return true;
}

function void Vulkan::ClearFrameBuffers()
{
    if (VulkanIsValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);
        for (u32 i = 0; i < vulkan.swapchainImageCount; i++)
        {
            if(VulkanIsValidHandle(vulkan.framebuffers[i]))
            {
                vkDestroyFramebuffer(vulkan.device, vulkan.framebuffers[i], 0);
                DebugPrint("Cleared FrameBuffer\n");
            }
        }
    }
}

function bool Vulkan::CreateCommandBuffers()
{
    ClearCommandBuffers();
    
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = vulkan.presentQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    ASSERT_VK_SUCCESS(vkCreateCommandPool(vulkan.device, &cmdPoolInfo, 0, &vulkan.cmdPool));
    
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.commandPool = vulkan.cmdPool;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = vulkan.swapchainImageCount;
    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, vulkan.cmdBuffers));
    
    for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
        vulkan.recordCmdBuffer[i] = true;
    DebugPrint("Created CommandBuffers\n");
    return true;
}

function void Vulkan::ClearCommandBuffers()
{
    if (VulkanIsValidHandle(vulkan.device) && VulkanIsValidHandle(vulkan.cmdPool))
    {
        vkDeviceWaitIdle(vulkan.device);
        vkResetCommandPool(vulkan.device, vulkan.cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        DebugPrint("Cleared Command Buffers\n");
    }
}

function bool Vulkan::CreateSwapchain()
{
    if (VulkanIsValidHandle(vulkan.device))
        vkDeviceWaitIdle(vulkan.device);
    else
        return false;
    
    vulkan.canRender = false;
    
    // NOTE: Get Surface capabilities
    {
        VkSurfaceCapabilitiesKHR surfCapabilities = {};
        ASSERT_VK_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.gpu, vulkan.surface, &surfCapabilities));
        
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
        if (VulkanIsValidHandle(oldSwapchain)) //old swapchain
            vkDestroySwapchainKHR(vulkan.device, oldSwapchain, 0);
    }
    
    // NOTE: Create the Image views
    {
        ClearSwapchainImages();
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
    DebugPrint("Created SwapChain\n");
    vulkan.canRender = true;
    
    return true;
}

function void Vulkan::ClearSwapchainImages()
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

function bool Vulkan::CreatePipeline()
{
    ClearPipeline();
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //vertexInputInfo.vertexBindingDescriptionCount = 0;
    //vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    //vertexInputInfo.vertexAttributeDescriptionCount = 0;
    //vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
    
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32) vulkan.windowExtent.width;
    viewport.height = (f32) vulkan.windowExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = vulkan.windowExtent;
    
    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;
    
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
    
    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.minSampleShading = 1.0f; // Optional
    //multisamplingInfo.pSampleMask = nullptr; // Optional
    //multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    //multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional
    
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachment.blendEnable = VK_FALSE;
    //blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    //blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    //blendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    //blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    //blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    //blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    
    VkPipelineColorBlendStateCreateInfo blendingInfo{};
    blendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendingInfo.logicOpEnable = VK_FALSE;
    blendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    blendingInfo.attachmentCount = 1;
    blendingInfo.pAttachments = &blendAttachment;
    //blendingInfo.blendConstants[0] = 0.0f; // Optional
    //blendingInfo.blendConstants[1] = 0.0f; // Optional
    //blendingInfo.blendConstants[2] = 0.0f; // Optional
    //blendingInfo.blendConstants[3] = 0.0f; // Optional
    
    /* 
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    [
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
     */
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.setLayoutCount = 0; // Optional
    //pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    //pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    //pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    
    ASSERT_VK_SUCCESS(vkCreatePipelineLayout(vulkan.device, &pipelineLayoutInfo, 0, &vulkan.pipelineLayout));
    
    // TODO(heyyod): THIS IS BAAAAADDDD! Change the paths
    VkShaderModule triangleVertShader, triangleFragShader = {};
    if(!LoadShader("..\\build\\shaders\\triangle.frag.spv", &triangleFragShader) ||
       !LoadShader("..\\build\\shaders\\triangle.vert.spv", &triangleVertShader))
    {
        Assert("Couldn't load shaders");
        return false;
    }
    
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
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = ArrayCount(shaderStages);
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    //pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &blendingInfo;
    //pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = vulkan.pipelineLayout;
    pipelineInfo.renderPass = vulkan.renderPass;
    pipelineInfo.subpass = 0;
    
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    
    ASSERT_VK_SUCCESS(vkCreateGraphicsPipelines(vulkan.device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &vulkan.pipeline));
    
    DebugPrint("Created Pipeline\n");
    vkDestroyShaderModule(vulkan.device, triangleVertShader, 0);
    vkDestroyShaderModule(vulkan.device, triangleFragShader, 0);
    return true;
}

function void Vulkan::ClearPipeline()
{
    if(VulkanIsValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);
        if (VulkanIsValidHandle(vulkan.pipeline))
        {
            vkDestroyPipeline(vulkan.device, vulkan.pipeline, 0);
            DebugPrint("Cleared Pipeline\n");
        }
        
        if (VulkanIsValidHandle(vulkan.pipelineLayout))
        {
            vkDestroyPipelineLayout(vulkan.device, vulkan.pipelineLayout, 0);
        }
        
    }
}

function void Vulkan::Destroy()
{
    if (VulkanIsValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);
        
        ClearPipeline();
        
        ClearFrameBuffers();
        if (VulkanIsValidHandle(vulkan.renderPass))
            vkDestroyRenderPass(vulkan.device, vulkan.renderPass, 0);
        
        if (VulkanIsValidHandle(vulkan.depthMemory))
            vkFreeMemory(vulkan.device, vulkan.depthMemory, 0);
        if (VulkanIsValidHandle(vulkan.depthImageView))
            vkDestroyImageView(vulkan.device, vulkan.depthImageView, 0);
        if (VulkanIsValidHandle(vulkan.depthImage))
            vkDestroyImage(vulkan.device, vulkan.depthImage, 0);
        
        ClearSwapchainImages();
        if (VulkanIsValidHandle(vulkan.swapchain))
            vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);
        
        if (VulkanIsValidHandle(vulkan.imageAvailableSem))
            vkDestroySemaphore(vulkan.device, vulkan.imageAvailableSem, 0);
        if (VulkanIsValidHandle(vulkan.renderFinishedSem))
            vkDestroySemaphore(vulkan.device, vulkan.renderFinishedSem, 0);
        if (VulkanIsValidHandle(vulkan.renderFence))
            vkDestroyFence(vulkan.device, vulkan.renderFence, 0);
        
        ClearCommandBuffers();
        
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

function bool Vulkan::Recreate()
{
    DebugPrint("\n-Recreate:\n");
    
    if (!CreateSwapchain())
        return false;
    if (!(vulkan.windowExtent.width == 0 || vulkan.windowExtent.height == 0))
    {
        if (!CreateCommandBuffers())
            return false;
        if (!CreateFrameBuffers())
            return false;
        if(!CreatePipeline())
            return false;
    }
    else
    {
        DebugPrint("Window minimized or completely shrinked.\n");
    }
    
    DebugPrint("\n");
    return true;
}

function bool Vulkan::ClearScreenToSolid(float color[3])
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = vulkan.renderPass;
    rpInfo.renderArea.extent = vulkan.windowExtent;
    rpInfo.clearValueCount = 1;
    
    VkClearValue clearValue = {};
    clearValue.color = VulkanClearColor(color[0], color[1], color[2], 0.0f);
    rpInfo.pClearValues = &clearValue;
    
    for (u32 i = 0; i < NUM_SWAPCHAIN_IMAGES; i++)
    {
        if (vulkan.recordCmdBuffer[i])
        {
            rpInfo.framebuffer = vulkan.framebuffers[i];
            //ASSERT_VK_SUCCESS(vkResetCommandBuffer(vulkan.cmdBuffers[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
            ASSERT_VK_SUCCESS(vkBeginCommandBuffer(vulkan.cmdBuffers[i], &commandBufferBeginInfo));
            vkCmdBeginRenderPass(vulkan.cmdBuffers[i], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(vulkan.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.pipeline);
            vkCmdDraw(vulkan.cmdBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(vulkan.cmdBuffers[i]);
            ASSERT_VK_SUCCESS(vkEndCommandBuffer(vulkan.cmdBuffers[i]));
        }
    }
    
    /*
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
    */
    return true;
}

function bool Vulkan::Draw()
{
    ASSERT_VK_SUCCESS(vkWaitForFences(vulkan.device, 1, &vulkan.renderFence, true, UINT64_MAX));
    ASSERT_VK_SUCCESS(vkResetFences(vulkan.device, 1, &vulkan.renderFence));
    
    // NOTE: Get the image
    VkResult result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX,
                                            vulkan.imageAvailableSem, 0, &vulkan.currentImage);
    
    if (vulkan.currentImage == 0)
    {
        vulkan.recordCmdBuffer[0] = false;
        vulkan.recordCmdBuffer[1] = true;
    }
    else
    {
        vulkan.recordCmdBuffer[0] = true;
        vulkan.recordCmdBuffer[1] = false;
    }
    
    switch (result)
    {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
        {
            break;
        }
        case VK_ERROR_OUT_OF_DATE_KHR:
        {
            return Recreate();
        }break;
        default:
        {
            DebugPrint("Problem occurred during swap chain image acquisition!\n");
            return false;
        }
    }
    
    // NOTE: Wait on imageAvailableSem and submit the command buffer and signal renderFinishedSem
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkan.imageAvailableSem;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkan.renderFinishedSem;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan.cmdBuffers[vulkan.currentImage];
    ASSERT_VK_SUCCESS(vkQueueSubmit(vulkan.graphicsQueue, 1, &submitInfo, vulkan.renderFence));
    
    // NOTE: Submit image to present when signaled by renderFinishedSem
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vulkan.renderFinishedSem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkan.swapchain;
    presentInfo.pImageIndices = &vulkan.currentImage;
    result = vkQueuePresentKHR(vulkan.presentQueue, &presentInfo);
    
    switch (result)
    {
        case VK_SUCCESS:
        {
            break;
        }
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
        {
            return Recreate();
        }
        default:
        {
            
            DebugPrint("Problem occurred during image presentation!\n");
            return false;
        }
    }
    return true;
}

function bool Vulkan::LoadShader(char *filepath, VkShaderModule *shaderOut)
{
    
    debug_read_file_result shaderCode = platformAPI.DEBUGReadFile(filepath);
    Assert(shaderCode.size < SHADER_CODE_BUFFER_SIZE);
    if(shaderCode.content)
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