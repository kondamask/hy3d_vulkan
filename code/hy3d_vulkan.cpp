#include "hy3d_vulkan.h"

#if VULKAN_VALIDATION_LAYERS_ON
function VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
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
function bool Vulkan::
LoadGlobalFunctions()
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
function bool Vulkan::
LoadInstanceFunctions()
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
    VulkanLoadInstanceFunc(vkGetPhysicalDeviceSurfacePresentModesKHR);
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
function bool Vulkan::
LoadDeviceFunctions()
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
    VulkanLoadDeviceFunc(vkCreateBuffer);
    VulkanLoadDeviceFunc(vkDestroyBuffer);
    VulkanLoadDeviceFunc(vkGetBufferMemoryRequirements);
    VulkanLoadDeviceFunc(vkBindBufferMemory);
    VulkanLoadDeviceFunc(vkMapMemory);
    VulkanLoadDeviceFunc(vkUnmapMemory);
    VulkanLoadDeviceFunc(vkFlushMappedMemoryRanges);
    VulkanLoadDeviceFunc(vkCreateDescriptorSetLayout);
    VulkanLoadDeviceFunc(vkDestroyDescriptorSetLayout);
    VulkanLoadDeviceFunc(vkCreateDescriptorPool);
    VulkanLoadDeviceFunc(vkDestroyDescriptorPool);
    VulkanLoadDeviceFunc(vkAllocateDescriptorSets);
    VulkanLoadDeviceFunc(vkUpdateDescriptorSets);
    
    VulkanLoadDeviceFunc(vkCmdBeginRenderPass);
    VulkanLoadDeviceFunc(vkCmdEndRenderPass);
    VulkanLoadDeviceFunc(vkCmdPipelineBarrier);
    VulkanLoadDeviceFunc(vkCmdClearColorImage);
    VulkanLoadDeviceFunc(vkCmdBindPipeline);
    VulkanLoadDeviceFunc(vkCmdBindVertexBuffers);
    VulkanLoadDeviceFunc(vkCmdBindIndexBuffer);
    VulkanLoadDeviceFunc(vkCmdDraw);
    VulkanLoadDeviceFunc(vkCmdDrawIndexed);
    VulkanLoadDeviceFunc(vkCmdSetViewport);
    VulkanLoadDeviceFunc(vkCmdSetScissor);
    VulkanLoadDeviceFunc(vkCmdCopyBuffer);
    VulkanLoadDeviceFunc(vkCmdBindDescriptorSets);
    
    DebugPrint("Loaded Device Functions\n");
    
    return true;
}

function bool Vulkan::
FindMemoryProperties(u32 reqMemType, VkMemoryPropertyFlags reqMemProperties, u32 &memoryIndexOut)
{
    // NOTE(heyyod): We assume we have already set the memory properties during creation.
    u32 memoryCount = vulkan.memoryProperties.memoryTypeCount;
    for (memoryIndexOut = 0; memoryIndexOut < memoryCount; ++memoryIndexOut)
    {
        uint32_t memoryType= (1 << memoryIndexOut);
        bool isRequiredMemoryType = reqMemType & memoryType;
        if(isRequiredMemoryType)
        {
            VkMemoryPropertyFlags properties = vulkan.memoryProperties.memoryTypes[memoryIndexOut].propertyFlags;
            bool hasRequiredProperties = ((properties & reqMemProperties) == reqMemProperties);
            if (hasRequiredProperties)
                return true;
        }
    }
    return false;
}

// TODO: make this cross-platform
function bool Vulkan::
Win32LoadDLL()
{
    vulkan.dll = LoadLibraryA("vulkan-1.dll");
    if (!vulkan.dll)
        return false;
    
    vkGetInstanceProcAddr = (vk_get_instance_proc_addr *)GetProcAddress(vulkan.dll, "vkGetInstanceProcAddr");
    
    DebugPrint("Loaded DLL\n");
    return true;
}


// TODO: Make it cross-platform
function bool Vulkan::
Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name)
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
    AssertSuccess(vkEnumerateInstanceLayerProperties(&layerCount, 0));
    
    VkLayerProperties availableLayers[16];
    Assert(layerCount <= ArrayCount(availableLayers));
    AssertSuccess(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));
    
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
#if VULKAN_VALIDATION_LAYERS_ON
        instanceInfo.enabledLayerCount = ArrayCount(validationLayers);
        instanceInfo.ppEnabledLayerNames = validationLayers;
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif
        
        AssertSuccess(vkCreateInstance(&instanceInfo, NULL, &vulkan.instance));
        DebugPrint("Created Vulkan Instance\n");
        
        if (!LoadInstanceFunctions())
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
            AssertSuccess(vkGetPhysicalDeviceSurfaceSupportKHR(vulkan.gpu, i, vulkan.surface, &supportsPresent[i]));
        
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
        
        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1; // NOTE: Only if graphicsQueueFamilyIndex == presentQueueFamilyIndex
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = ArrayCount(desiredDeviceExtensions);
        deviceInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
        AssertSuccess(vkCreateDevice(vulkan.gpu, &deviceInfo, 0, &vulkan.device));
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
        AssertSuccess(vkCreateImage(vulkan.device, &depthImageInfo, 0, &vulkan.depthImage));
        vkGetImageMemoryRequirements(vulkan.device, vulkan.depthImage, &depthMemoryReq);

        u32 memoryIndex = 0; // no use for now
        if (!FindMemoryProperties(vulkan.memoryProperties, depthMemoryReq.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryIndex))
            return false;

        VkMemoryAllocateInfo depthMemAllocInfo = {};
        depthMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        depthMemAllocInfo.allocationSize = depthMemoryReq.size;

        AssertSuccess(vkAllocateMemory(vulkan.device, &depthMemAllocInfo, 0, &vulkan.depthMemory));
        AssertSuccess(vkBindImageMemory(vulkan.device, vulkan.depthImage, vulkan.depthMemory, 0));

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
        AssertSuccess(vkCreateImageView(vulkan.device, &depthImageViewInfo, 0, &vulkan.depthImageView));
    }*/
    
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
        //subpassDescriptions.inputAttachmentCount = 0;
        //subpassDescriptions.pInputAttachments = 0;
        
        // NOTE(heyyod): The index of the color attachment in this array is directly 
        //referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
        subpassDescriptions.colorAttachmentCount = 1; // ArrayCount(colorAttachementRef)
        subpassDescriptions.pColorAttachments = &colorAttachementRef;
        
        //subpassDescriptions.pResolveAttachments = 0;
        //subpassDescriptions.pDepthStencilAttachment = 0;
        //subpassDescriptions.preserveAttachmentCount = 0;
        //subpassDescriptions.pPreserveAttachments = 0;
        
        VkSubpassDependency renderpassDependencies[2] = {};;
        renderpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        renderpassDependencies[0].dstSubpass = 0;
        renderpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        renderpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        renderpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        renderpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        renderpassDependencies[1].srcSubpass = 0;
        renderpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        renderpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        renderpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        renderpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        renderpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        
        
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescriptions;
        renderPassInfo.dependencyCount = ArrayCount(renderpassDependencies);
        renderPassInfo.pDependencies = renderpassDependencies;
        AssertSuccess(vkCreateRenderPass(vulkan.device, &renderPassInfo, 0, &vulkan.renderPass));
        DebugPrint("Created a renderpass\n");
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
        
        
        for(u32 i = 0; i < NUM_RESOURCES; i++)
        {
            AssertSuccess(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, &vulkan.resources[i].cmdBuffer));
            AssertSuccess(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.resources[i].imgAvailableSem));
            AssertSuccess(vkCreateSemaphore(vulkan.device, &semaphoreInfo, 0, &vulkan.resources[i].frameReadySem));
            AssertSuccess(vkCreateFence(vulkan.device, &fenceInfo, 0, &vulkan.resources[i].fence));
        }
        DebugPrint("Created rendering resources.\n");
    }
    
    // NOTE(heyyod): Make a buffers
    {
        u32 vertexBufferSize = MEGABYTES(100);
        u32 indexBufferSize = MEGABYTES(100);
        u32 stagingBufferSize = vertexBufferSize + indexBufferSize;
        if(!CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferSize, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vulkan.stagingBuffer, MAP_BUFFER_TRUE))
        {
            Assert("Could not create staging buffer");
            return false;
        }
        
        if(!CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan.vertexBuffer))
        {
            Assert("Could not create vertex buffer");
            return false;
        }
        
        if(!CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan.indexBuffer))
        {
            Assert("Could not create index buffer");
            return false;
        }
    }
    
    // NOTE(heyyod): Create uniform buffers and their descriptor 
    {
        //model, view and projection matrices
        for (u32 i = 0; i < ArrayCount(vulkan.mvp); i++)
        {
            if (!CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(model_view_proj), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkan.mvp[i], MAP_BUFFER_TRUE))
            {
                Assert("Could not create uniform buffers");
                return false;
            }
        }
        
        VkDescriptorSetLayoutBinding mvpDescSetLayoutBinding = {};
        mvpDescSetLayoutBinding.binding = 0;
        mvpDescSetLayoutBinding.descriptorCount = 1;
        mvpDescSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpDescSetLayoutBinding.pImmutableSamplers = 0;
        mvpDescSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        
        VkDescriptorSetLayoutCreateInfo mvpDescSetLayoutInfo = {};
        mvpDescSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        mvpDescSetLayoutInfo.bindingCount = 1;
        mvpDescSetLayoutInfo.pBindings = &mvpDescSetLayoutBinding;
        AssertSuccess(vkCreateDescriptorSetLayout(vulkan.device, &mvpDescSetLayoutInfo, 0, &vulkan.mvpDescSetLayout));
        
        
        VkDescriptorPoolSize mvpDescPoolSize = {};
        mvpDescPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpDescPoolSize.descriptorCount = NUM_UNIFORM_BUFFERS;
        
        VkDescriptorPoolCreateInfo mvpDescPoolInfo = {};
        mvpDescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        mvpDescPoolInfo.poolSizeCount = 1;
        mvpDescPoolInfo.pPoolSizes = &mvpDescPoolSize;
        mvpDescPoolInfo.maxSets = NUM_UNIFORM_BUFFERS;
        AssertSuccess(vkCreateDescriptorPool(vulkan.device, &mvpDescPoolInfo, 0, &vulkan.mvpDescPool));
        
        VkDescriptorSetLayout mvpDescSetLayouts[NUM_UNIFORM_BUFFERS] = {
            vulkan.mvpDescSetLayout, vulkan.mvpDescSetLayout};
        
        VkDescriptorSetAllocateInfo mvpDescAllocInfo = {};
        mvpDescAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mvpDescAllocInfo.descriptorPool = vulkan.mvpDescPool;
        mvpDescAllocInfo.descriptorSetCount = ArrayCount(mvpDescSetLayouts);
        mvpDescAllocInfo.pSetLayouts = mvpDescSetLayouts;
        AssertSuccess(vkAllocateDescriptorSets(vulkan.device, &mvpDescAllocInfo, vulkan.mvpDescSets));
        
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(model_view_proj); // VK_WHOLE_SIZE
        
        VkWriteDescriptorSet mvpDescWrite = {};
        mvpDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpDescWrite.dstBinding = 0;
        mvpDescWrite.dstArrayElement = 0;
        mvpDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpDescWrite.descriptorCount = 1;
        mvpDescWrite.pBufferInfo = &bufferInfo;
        
        for (u8 i = 0; i < NUM_UNIFORM_BUFFERS; i++)
        {
            bufferInfo.buffer = vulkan.mvp[i].handle;
            mvpDescWrite.dstSet = vulkan.mvpDescSets[i];
            vkUpdateDescriptorSets(vulkan.device, 1, &mvpDescWrite, 0, 0);
        }
    }
    
    
    
    if(!CreatePipeline())
    {
        Assert("Could not create pipeline");
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

function void Vulkan::
ClearFrameBuffers()
{
    if (VulkanIsValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);
        for (u32 i = 0; i < NUM_RESOURCES; i++)
        {
            if(VulkanIsValidHandle(vulkan.resources[i].framebuffer))
            {
                vkDestroyFramebuffer(vulkan.device, vulkan.resources[i].framebuffer, 0);
                DebugPrint("Cleared FrameBuffer\n");
            }
        }
    }
}

function bool Vulkan::
CreateSwapchain()
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
            desiredPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; //VK_PRESENT_MODE_FIFO_KHR;
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
    vulkan.canRender = true;
    
    return true;
}

function void Vulkan::
ClearSwapchainImages()
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

function bool Vulkan::
LoadShader(char *filepath, VkShaderModule *shaderOut)
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

function bool Vulkan::
CreatePipeline()
{
    ClearPipeline();
    
    VkShaderModule triangleVertShader = {};
    VkShaderModule triangleFragShader = {};
    if(!LoadShader("..\\build\\shaders\\triangle.frag.spv", &triangleFragShader) ||
       !LoadShader("..\\build\\shaders\\triangle.vert.spv", &triangleVertShader))
    {
        Assert("Couldn't load shaders");
        return false;
    }
    
    // NOTE(heyyod): specify some general info for the memory of
    // the vertex buffer we'll be reading from
    VkVertexInputBindingDescription vertexBindingDesc;
    vertexBindingDesc.binding = 0;
    vertexBindingDesc.stride = sizeof(vertex2);
    vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    VkVertexInputAttributeDescription vertexAttributeDescs[2];
    // NOTE(heyyod): specify the inPosition format in vertex shader
    vertexAttributeDescs[0].binding = 0;
    vertexAttributeDescs[0].location = 0;// NOTE(heyyod): this maches the value in the shader
    vertexAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescs[0].offset = offsetof(vertex2, pos);
    // NOTE(heyyod): spesify the inColor format in vertex shader
    vertexAttributeDescs[1].binding = 0;
    vertexAttributeDescs[1].location = 1;
    vertexAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescs[1].offset = offsetof(vertex2, color);
    
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
    
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    
    //-
    
    /* NOTE(heyyod): We can make these dynamic and update them with vkCmd commands 
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
         */
    
    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    //viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    //viewportInfo.pScissors = &scissor;
    
    //-
    
    VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //rasterizerInfo.depthClampEnable = VK_FALSE;
    //rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //rasterizerInfo.depthBiasEnable = VK_FALSE;
    //rasterizerInfo.depthBiasConstantFactor = 0.0f; // Optional
    //rasterizerInfo.depthBiasClamp = 0.0f; // Optional
    //rasterizerInfo.depthBiasSlopeFactor = 0.0f; // Optional
    
    //-
    
    VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.minSampleShading = 1.0f; // Optional
    //multisamplingInfo.pSampleMask = nullptr; // Optional
    //multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    //multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional
    
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
    pipelineLayoutInfo.pSetLayouts = &vulkan.mvpDescSetLayout;
    //pipelineLayoutInfo.pushConstantRangeCount = 0;
    //pipelineLayoutInfo.pPushConstantRanges = nullptr;
    AssertSuccess(vkCreatePipelineLayout(vulkan.device, &pipelineLayoutInfo, 0, &vulkan.pipelineLayout));
    
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
    pipelineInfo.pDynamicState = &dynamicStateInfo; // Optional
    pipelineInfo.layout = vulkan.pipelineLayout;
    pipelineInfo.renderPass = vulkan.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    AssertSuccess(vkCreateGraphicsPipelines(vulkan.device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &vulkan.pipeline));
    
    DebugPrint("Created Pipeline\n");
    vkDestroyShaderModule(vulkan.device, triangleVertShader, 0);
    vkDestroyShaderModule(vulkan.device, triangleFragShader, 0);
    return true;
}

function void Vulkan::
ClearPipeline()
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


#define CreateVertexBuffer(usage, nVertices, vertexType, properties, bufferOut) \
CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usage, sizeof(vertexType) * nVertices, properties, bufferOut)

function bool Vulkan::
CreateBuffer(VkBufferUsageFlags usage, u64 size, VkMemoryPropertyFlags properties, vulkan_buffer &bufferOut, bool mapBuffer)
{
    if(!(VulkanIsValidHandle(bufferOut.handle)))
    {
        bufferOut.size = size;
        
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = usage;
        bufferInfo.size = size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        AssertSuccess(vkCreateBuffer(vulkan.device, &bufferInfo, 0, &bufferOut.handle));
        
        VkMemoryRequirements memoryReq= {};
        vkGetBufferMemoryRequirements(vulkan.device, bufferOut.handle, &memoryReq);
        
        u32 memIndex = 0;
        if(Vulkan::FindMemoryProperties(memoryReq.memoryTypeBits, properties, memIndex))
        {
            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memoryReq.size;
            allocInfo.memoryTypeIndex = memIndex;
            
            AssertSuccess(vkAllocateMemory(vulkan.device, &allocInfo, 0, &bufferOut.memoryHandle));
            AssertSuccess(vkBindBufferMemory(vulkan.device, bufferOut.handle, bufferOut.memoryHandle, 0));
            
            if(mapBuffer)
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

function void Vulkan::
ClearBuffer(vulkan_buffer buffer)
{
    if(VulkanIsValidHandle(vulkan.device))
    {
        if(VulkanIsValidHandle(buffer.handle))
            vkDestroyBuffer(vulkan.device, buffer.handle, 0);
        if(VulkanIsValidHandle(buffer.memoryHandle))
        {
            vkFreeMemory(vulkan.device, buffer.memoryHandle, 0);
        }
    }
}

function void Vulkan::
Destroy()
{
    if (VulkanIsValidHandle(vulkan.device))
    {
        vkDeviceWaitIdle(vulkan.device);
        
        ClearPipeline();
        
        if(VulkanIsValidHandle(vulkan.mvpDescSetLayout))
        {
            vkDestroyDescriptorSetLayout(vulkan.device, vulkan.mvpDescSetLayout, 0);
        }
        
        for(u32 i = 0; i < ArrayCount(vulkan.mvp); i++)
        {
            ClearBuffer(vulkan.mvp[i]);
        }
        
        if(VulkanIsValidHandle(vulkan.mvpDescPool))
            vkDestroyDescriptorPool(vulkan.device, vulkan.mvpDescPool, 0);
        
        vkUnmapMemory(vulkan.device, vulkan.stagingBuffer.memoryHandle);
        ClearBuffer(vulkan.stagingBuffer);
        ClearBuffer(vulkan.vertexBuffer);
        ClearBuffer(vulkan.indexBuffer);
        
        ClearFrameBuffers();
        if (VulkanIsValidHandle(vulkan.renderPass))
            vkDestroyRenderPass(vulkan.device, vulkan.renderPass, 0);
        
        /* 
                if (VulkanIsValidHandle(vulkan.depthMemory))
                    vkFreeMemory(vulkan.device, vulkan.depthMemory, 0);
                if (VulkanIsValidHandle(vulkan.depthImageView))
                    vkDestroyImageView(vulkan.device, vulkan.depthImageView, 0);
                if (VulkanIsValidHandle(vulkan.depthImage))
                    vkDestroyImage(vulkan.device, vulkan.depthImage, 0);
                 */
        
        ClearSwapchainImages();
        if (VulkanIsValidHandle(vulkan.swapchain))
            vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);
        
        for(u32 i = 0; i < NUM_RESOURCES; i++)
        {
            if (VulkanIsValidHandle(vulkan.resources[i].imgAvailableSem))
                vkDestroySemaphore(vulkan.device, vulkan.resources[i].imgAvailableSem, 0);
            if (VulkanIsValidHandle(vulkan.resources[i].frameReadySem))
                vkDestroySemaphore(vulkan.device, vulkan.resources[i].frameReadySem, 0);
            if (VulkanIsValidHandle(vulkan.resources[i].fence))
                vkDestroyFence(vulkan.device, vulkan.resources[i].fence, 0);
        }
        
        if(VulkanIsValidHandle(vulkan.cmdPool))
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

function bool Vulkan::
Draw(update_data *data)
{
    // NOTE(heyyod): Get the resource tha we'll use to prepare the next frame
    frame_prep_resource *renderingRes = &vulkan.resources[vulkan.currentResource];
    // NOTE(heyyod): update the currentResource for the next call (next draw loop)
    vulkan.currentResource = (vulkan.currentResource + 1) % NUM_RESOURCES;
    
    // NOTE(heyyod): Wait for the resource to be available;
    AssertSuccess(vkWaitForFences(vulkan.device, 1, &renderingRes->fence, true, UINT64_MAX));
    AssertSuccess(vkResetFences(vulkan.device, 1, &renderingRes->fence));
    
    if (data->updateVertexBuffer)
    {
        VkMappedMemoryRange flushRange = {};
        flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE; 
        flushRange.memory = vulkan.stagingBuffer.memoryHandle;
        flushRange.size = vulkan.stagingBuffer.size;
        
        vkFlushMappedMemoryRanges(vulkan.device, 1, &flushRange );
        
        // NOTE(heyyod): copy staging buffer to vertex buffer
        VkCommandBufferBeginInfo cmdBufferBeginInfo= {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        AssertSuccess(vkBeginCommandBuffer(renderingRes->cmdBuffer, &cmdBufferBeginInfo));
        
        u64 meshSize = 0;
        u64 verticesSize = 0;
        u64 indicesSize = 0;
        VkBufferCopy vertexBufferCopyInfo = {};
        VkBufferCopy indexBufferCopyInfo = {};
        //for(u32 meshId = 0; meshId < 1; meshId++)
        for(u32 meshId = 0; meshId < ArrayCount(data->meshes); meshId++)
        {
            meshSize = MESH_TOTAL_SIZE(data->meshes[meshId]);
            verticesSize =  MESH_VERTICES_SIZE(data->meshes[meshId]);
            indicesSize =  MESH_INDICES_SIZE(data->meshes[meshId]);
            
            if (meshId > 0)
            {
                vertexBufferCopyInfo.srcOffset += meshSize;
                vertexBufferCopyInfo.dstOffset += verticesSize;
                indexBufferCopyInfo.dstOffset += indicesSize;
            }
            indexBufferCopyInfo.srcOffset = vertexBufferCopyInfo.srcOffset + verticesSize;
            vertexBufferCopyInfo.size = verticesSize;
            indexBufferCopyInfo.size = indicesSize;
            
            vkCmdCopyBuffer(renderingRes->cmdBuffer, vulkan.stagingBuffer.handle, vulkan.vertexBuffer.handle, 1, &vertexBufferCopyInfo);
            vkCmdCopyBuffer(renderingRes->cmdBuffer, vulkan.stagingBuffer.handle, vulkan.indexBuffer.handle, 1, &indexBufferCopyInfo);
        }
        
        VkBufferMemoryBarrier bufferMemoryBarrier = {};
        bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        bufferMemoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferMemoryBarrier.buffer = vulkan.vertexBuffer.handle;
        bufferMemoryBarrier.offset = 0; 
        bufferMemoryBarrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(renderingRes->cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, 0, 1, &bufferMemoryBarrier, 0, 0);
        AssertSuccess(vkEndCommandBuffer(renderingRes->cmdBuffer));
        
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &renderingRes->cmdBuffer;
        
        AssertSuccess(vkQueueSubmit(vulkan.graphicsQueue, 1, &submitInfo, renderingRes->fence));
        
        AssertSuccess(vkWaitForFences(vulkan.device, 1, &renderingRes->fence, true, UINT64_MAX));
        AssertSuccess(vkResetFences(vulkan.device, 1, &renderingRes->fence));
        
        //memset(vulkan.stagingBuffer.data, 0, vulkan.stagingBuffer.size);
        data->updateVertexBuffer = false;
    }
    
    
    // NOTE: Get the next image that we'll use to create the frame and draw it
    // Signal the semaphore when it's available
    local u32 nextImage = 0;
    local VkResult result = {}; 
    {
        result = vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX, renderingRes->imgAvailableSem, 0, &nextImage);
        
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
        local VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.width = vulkan.windowExtent.width;
        framebufferInfo.height = vulkan.windowExtent.height;
        framebufferInfo.layers = 1;
        framebufferInfo.pAttachments = &vulkan.swapchainImageViews[nextImage];
        if(VulkanIsValidHandle(renderingRes->framebuffer))
        {
            vkDestroyFramebuffer(vulkan.device, renderingRes->framebuffer, 0);
            renderingRes->framebuffer = VK_NULL_HANDLE;
        }
        AssertSuccess(vkCreateFramebuffer(vulkan.device, &framebufferInfo, 0, &renderingRes->framebuffer));
        
        local VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        local VkClearValue clearValue = {};
        clearValue.color = VulkanClearColor(data->clearColor[0], data->clearColor[1], data->clearColor[2], 0.0f);
        
        local VkRenderPassBeginInfo renderpassInfo = {};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassInfo.renderPass = vulkan.renderPass;
        renderpassInfo.renderArea.extent = vulkan.windowExtent;
        renderpassInfo.clearValueCount = 1;
        renderpassInfo.pClearValues = &clearValue;
        renderpassInfo.framebuffer = renderingRes->framebuffer;
        
        local VkViewport viewport = {};
        //viewport.x = 0.0f;
        //viewport.y = 0.0f;
        viewport.width = (f32) vulkan.windowExtent.width;
        viewport.height = (f32) vulkan.windowExtent.height;
        //viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        local VkRect2D scissor = {};
        //scissor.offset = {0, 0};
        scissor.extent = vulkan.windowExtent;
        
        local VkDeviceSize bufferOffset = 0;
        
        AssertSuccess(vkBeginCommandBuffer(renderingRes->cmdBuffer, &commandBufferBeginInfo));
        vkCmdBeginRenderPass(renderingRes->cmdBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderingRes->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.pipeline);
        vkCmdSetViewport(renderingRes->cmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor(renderingRes->cmdBuffer, 0, 1, &scissor);
        
        vkCmdBindVertexBuffers(renderingRes->cmdBuffer, 0, 1, &vulkan.vertexBuffer.handle, &bufferOffset);
        vkCmdBindIndexBuffer(renderingRes->cmdBuffer, vulkan.indexBuffer.handle, 0, VULKAN_INDEX_TYPE);
        vkCmdBindDescriptorSets(renderingRes->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan.pipelineLayout, 0, 1, &vulkan.mvpDescSets[nextImage], 0, 0);
        
        u32 firstIndex = 0;
        u32 indexOffset = 0;
        for(u32 meshId = 0; meshId < ArrayCount(data->meshes); meshId++)
        {
            vkCmdDrawIndexed(renderingRes->cmdBuffer, data->meshes[meshId].nIndices, 1,
                             firstIndex, indexOffset, 0);
            
            firstIndex += data->meshes[meshId].nIndices;
            indexOffset += data->meshes[meshId].nVertices;
        }
        
        
        vkCmdEndRenderPass(renderingRes->cmdBuffer);
        AssertSuccess(vkEndCommandBuffer(renderingRes->cmdBuffer));
    }
    
    // NOTE: Wait on imageAvailableSem and submit the command buffer and signal renderFinishedSem
    {
        local VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        local VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitDestStageMask;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &renderingRes->imgAvailableSem;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderingRes->frameReadySem;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &renderingRes->cmdBuffer;
        AssertSuccess(vkQueueSubmit(vulkan.graphicsQueue, 1, &submitInfo, renderingRes->fence));
        
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
        local VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderingRes->frameReadySem;
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
        u32 nextUniformBuffer = (nextImage + 1) % NUM_UNIFORM_BUFFERS;
        data->newMVP = vulkan.mvp[nextUniformBuffer].data;
    }
    
    return true;
}

#if 0
function bool Vulkan::
CreateCommandBuffers()
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
    for(u32 i = 0; i < NUM_RESOURCES; i++)
    {
        AssertSuccess(vkAllocateCommandBuffers(vulkan.device, &cmdBufferAllocInfo, &vulkan.resources[i].cmdBuffer));
    }
    DebugPrint("Created Command Pool and Command Buffers\n");
    
    return true;
}


function bool Vulkan::
ResetCommandBuffers()
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

function void Vulkan::
GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc)
{
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(v);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

function void Vulkan::
GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs)
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