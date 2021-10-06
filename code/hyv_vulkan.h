#pragma once
#include "hyv_base.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <D:/.DEV/VulkanSDK/1.2.189.2/Include/vulkan/vulkan.h>

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

    VkSwapchainKHR swapchain;
    u32 swapchainImageCount;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[NUM_SWAPCHAIN_IMAGES];

    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffers[NUM_SWAPCHAIN_IMAGES];

    VkFormat depthFormat;
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthMemory;

    bool canRender;

    HMODULE dll;

#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

namespace Vulkan
{
    static bool Win32LoadDLL(vulkan_state &vulkan);
    static bool Win32Initialize(vulkan_state &vulkan, HINSTANCE &wndInstance, HWND &wndHandle, const char *name);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

    static bool LoadGlobalFunctions();
    static bool LoadInstanceFunctions(VkInstance instance);
    static bool LoadDeviceFunctions(VkDevice device);
    static bool FindMemoryProperties(VkPhysicalDeviceMemoryProperties &memoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndex);
    static bool CreateCommandBuffers(vulkan_state &vulkan);
    static bool CreateSwapchain(vulkan_state &vulkan);
    static void ClearCommands(vulkan_state &vulkan);
    static void ClearSwapchainImages(vulkan_state &vulkan);
    static void Destroy(vulkan_state &vulkan);
    static bool ClearScreen(vulkan_state &vulkan, float r = 0.8f);
    static bool OnWindowSizeChange(vulkan_state &vulkan);
    static bool Draw(vulkan_state &vulkan);
    static bool CanRender(vulkan_state &vulkan);
}

//=====================================================================
// NOTE: Macros
#define VulkanFuncPtr(func) PFN_##func
#define VulkanDeclareFunction(func) static VulkanFuncPtr(func) func
#define VulkanClearColor(r, g, b, a) {r, g, b, a}
#define VulkanValidHandle(obj) obj != VK_NULL_HANDLE

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

