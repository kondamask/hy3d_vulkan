#pragma once
#include "hy3d_base.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES

// TODO: WHY IS THE INCLUDE DIR NOT INCLUDED BY THE COMPILER????
#include <D:/.DEV/VulkanSDK/1.2.176.1/Include/vulkan/vulkan.h>

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define NUM_SWAPCHAIN_IMAGES 2

struct vulkan_state
{
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;

    u32 graphicsQueueFamilyIndex;
    u32 presentQueueFamilyIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSemaphore imageAvailableSem;
    VkSemaphore renderFinishedSem;
    VkFence renderFence;

    u32 currentImage;
    VkExtent2D windowExtent;
    VkSwapchainKHR swapchain;
    u32 swapchainImageCount;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[NUM_SWAPCHAIN_IMAGES];

    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffers[NUM_SWAPCHAIN_IMAGES];
    bool recordCmdBuffer[NUM_SWAPCHAIN_IMAGES];

    VkFormat depthFormat;
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthMemory;

    VkRenderPass renderPass;
    VkFramebuffer framebuffers[NUM_SWAPCHAIN_IMAGES];

    bool canRender;

    HMODULE dll;

#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};
static vulkan_state vulkan;

namespace Vulkan
{
    static bool Win32LoadDLL();
    static bool Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name);

    static bool LoadGlobalFunctions();
    static bool LoadInstanceFunctions();
    static bool LoadDeviceFunctions();

    static bool CreateSwapchain();
    static bool CreateCommandBuffers();
    static bool CreateFrameBuffers();

    static void ClearCommandBuffers();
    static void ClearFrameBuffers();
    static void ClearSwapchainImages();
    static void Destroy();

    static bool Draw();

    static bool ClearScreenToSolid(f32 color[3]);
    static bool OnWindowSizeChange();
    static bool FindMemoryProperties(VkPhysicalDeviceMemoryProperties &memoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndex);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
}

//=====================================================================
// NOTE: Macros
#define VulkanFuncPtr(func) PFN_##func
#define VulkanDeclareFunction(func) static VulkanFuncPtr(func) func
#define VulkanClearColor(r, g, b, a) \
    {                                \
        r, g, b, a                   \
    }
#define VulkanIsValidHandle(obj) obj != VK_NULL_HANDLE

//=====================================================================
// NOTE: Global Functions
#define VK_GET_INSTANCE_PROC_ADDR(name) PFN_vkVoidFunction name(VkInstance instance, const char *pName)
typedef VK_GET_INSTANCE_PROC_ADDR(vk_get_instance_proc_addr);
VK_GET_INSTANCE_PROC_ADDR(vkGetInstanceProcAddrStub) { return 0; }
static vk_get_instance_proc_addr *vkGetInstanceProcAddr_ = vkGetInstanceProcAddrStub;
#define vkGetInstanceProcAddr vkGetInstanceProcAddr_

VulkanDeclareFunction(vkCreateInstance);
VulkanDeclareFunction(vkEnumerateInstanceLayerProperties);
VulkanDeclareFunction(vkEnumerateInstanceExtensionProperties);

//......................................................................

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

    DebugPrint("Loaded Global Functions\n");

    return true;
}

//=====================================================================
// NOTE: Instance Functions
VulkanDeclareFunction(vkDestroyInstance);

#if VULKAN_VALIDATION_LAYERS_ON
VulkanDeclareFunction(vkCreateDebugUtilsMessengerEXT);
VulkanDeclareFunction(vkDestroyDebugUtilsMessengerEXT);
#endif

VulkanDeclareFunction(vkCreateWin32SurfaceKHR); // NOTE: Windows ONLY
VulkanDeclareFunction(vkDestroySurfaceKHR);
VulkanDeclareFunction(vkEnumeratePhysicalDevices);
VulkanDeclareFunction(vkGetPhysicalDeviceQueueFamilyProperties);
VulkanDeclareFunction(vkGetPhysicalDeviceSurfaceSupportKHR);
VulkanDeclareFunction(vkGetPhysicalDeviceSurfaceFormatsKHR);
VulkanDeclareFunction(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
VulkanDeclareFunction(vkGetPhysicalDeviceFormatProperties);
VulkanDeclareFunction(vkGetPhysicalDeviceMemoryProperties);
VulkanDeclareFunction(vkEnumerateDeviceExtensionProperties);
VulkanDeclareFunction(vkCreateDevice);
VulkanDeclareFunction(vkGetDeviceProcAddr);

//......................................................................

#define VulkanLoadInstanceFunc(func)                                           \
    func = (VulkanFuncPtr(func))vkGetInstanceProcAddr(vulkan.instance, #func); \
    if (!(func))                                                               \
    {                                                                          \
        return false;                                                          \
    }
static bool Vulkan::LoadInstanceFunctions()
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

//=====================================================================
// NOTE: Device Functions
VulkanDeclareFunction(vkDeviceWaitIdle);
VulkanDeclareFunction(vkDestroyDevice);
VulkanDeclareFunction(vkGetDeviceQueue);
VulkanDeclareFunction(vkCreateSwapchainKHR);
VulkanDeclareFunction(vkDestroySwapchainKHR);
VulkanDeclareFunction(vkGetSwapchainImagesKHR);
VulkanDeclareFunction(vkCreateSemaphore);
VulkanDeclareFunction(vkDestroySemaphore);
VulkanDeclareFunction(vkAcquireNextImageKHR);
VulkanDeclareFunction(vkQueueSubmit);
VulkanDeclareFunction(vkQueuePresentKHR);
VulkanDeclareFunction(vkCreateCommandPool);
VulkanDeclareFunction(vkDestroyCommandPool);
VulkanDeclareFunction(vkAllocateCommandBuffers);
VulkanDeclareFunction(vkFreeCommandBuffers);
VulkanDeclareFunction(vkBeginCommandBuffer);
VulkanDeclareFunction(vkEndCommandBuffer);
VulkanDeclareFunction(vkResetCommandBuffer);
VulkanDeclareFunction(vkCmdPipelineBarrier);
VulkanDeclareFunction(vkCmdClearColorImage);
VulkanDeclareFunction(vkCreateImage);
VulkanDeclareFunction(vkDestroyImage);
VulkanDeclareFunction(vkCreateImageView);
VulkanDeclareFunction(vkDestroyImageView);
VulkanDeclareFunction(vkGetImageMemoryRequirements);
VulkanDeclareFunction(vkBindImageMemory);
VulkanDeclareFunction(vkAllocateMemory);
VulkanDeclareFunction(vkFreeMemory);
VulkanDeclareFunction(vkCreateRenderPass);
VulkanDeclareFunction(vkDestroyRenderPass);
VulkanDeclareFunction(vkCreateFramebuffer);
VulkanDeclareFunction(vkDestroyFramebuffer);
VulkanDeclareFunction(vkCreateFence);
VulkanDeclareFunction(vkDestroyFence);
VulkanDeclareFunction(vkWaitForFences);
VulkanDeclareFunction(vkResetFences);
VulkanDeclareFunction(vkCmdBeginRenderPass);
VulkanDeclareFunction(vkCmdEndRenderPass);

//......................................................................

#define VulkanLoadDeviceFunc(func)                                         \
    func = (VulkanFuncPtr(func))vkGetDeviceProcAddr(vulkan.device, #func); \
    if (!(func))                                                           \
    {                                                                      \
        return false;                                                      \
    }
static bool Vulkan::LoadDeviceFunctions()
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
    VulkanLoadDeviceFunc(vkResetCommandBuffer);
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
    VulkanLoadDeviceFunc(vkCreateRenderPass);
    VulkanLoadDeviceFunc(vkDestroyRenderPass);
    VulkanLoadDeviceFunc(vkCreateFramebuffer);
    VulkanLoadDeviceFunc(vkDestroyFramebuffer);
    VulkanLoadDeviceFunc(vkCreateFence);
    VulkanLoadDeviceFunc(vkDestroyFence);
    VulkanLoadDeviceFunc(vkWaitForFences);
    VulkanLoadDeviceFunc(vkResetFences);
    VulkanLoadDeviceFunc(vkCmdBeginRenderPass);
    VulkanLoadDeviceFunc(vkCmdEndRenderPass);

    DebugPrint("Loaded Device Functions\n");

    return true;
}
