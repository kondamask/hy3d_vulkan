#pragma once
#include "hyv_base.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

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

    VkQueue queue;
    //VkQueue graphicsQueue;
    //VkQueue presentQueue;

    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;

    VkSwapchainKHR swapchain;
    u32 swapchainImageCount;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView imageViews[NUM_SWAPCHAIN_IMAGES];

    VkFormat depthFormat;
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthMemory;

    HMODULE dll;

#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

#define VK_GET_INSTANCE_PROC_ADDR(name) PFN_vkVoidFunction name(VkInstance instance, const char *pName)
typedef VK_GET_INSTANCE_PROC_ADDR(vk_get_instance_proc_addr);
VK_GET_INSTANCE_PROC_ADDR(vkGetInstanceProcAddrStub) { return 0; }
static vk_get_instance_proc_addr *vkGetInstanceProcAddr_ = vkGetInstanceProcAddrStub;
#define vkGetInstanceProcAddr vkGetInstanceProcAddr_

#define VK_DECLARE_FUNCTION(fun) \
    PFN_##fun fun

// NOTE: Global Functions
VK_DECLARE_FUNCTION(vkCreateInstance);
VK_DECLARE_FUNCTION(vkEnumerateInstanceLayerProperties);

// NOTE: Instance Functions
VK_DECLARE_FUNCTION(vkCreateDebugUtilsMessengerEXT);
VK_DECLARE_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
VK_DECLARE_FUNCTION(vkCreateWin32SurfaceKHR);
VK_DECLARE_FUNCTION(vkDestroySurfaceKHR);
VK_DECLARE_FUNCTION(vkEnumeratePhysicalDevices);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceFormatProperties);
VK_DECLARE_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
VK_DECLARE_FUNCTION(vkCreateDevice);
VK_DECLARE_FUNCTION(vkGetDeviceProcAddr);
VK_DECLARE_FUNCTION(vkDestroyInstance);

// NOTE: Device Functions
VK_DECLARE_FUNCTION(vkGetDeviceQueue);
VK_DECLARE_FUNCTION(vkCreateCommandPool);
VK_DECLARE_FUNCTION(vkAllocateCommandBuffers);
VK_DECLARE_FUNCTION(vkCreateSwapchainKHR);
VK_DECLARE_FUNCTION(vkGetSwapchainImagesKHR);
VK_DECLARE_FUNCTION(vkCreateImageView);
VK_DECLARE_FUNCTION(vkCreateImage);
VK_DECLARE_FUNCTION(vkDestroyImage);
VK_DECLARE_FUNCTION(vkDestroyImageView);
VK_DECLARE_FUNCTION(vkDestroySwapchainKHR);
VK_DECLARE_FUNCTION(vkDestroyCommandPool);
VK_DECLARE_FUNCTION(vkDeviceWaitIdle);
VK_DECLARE_FUNCTION(vkDestroyDevice);
VK_DECLARE_FUNCTION(vkGetImageMemoryRequirements);
VK_DECLARE_FUNCTION(vkAllocateMemory);
VK_DECLARE_FUNCTION(vkFreeMemory);
VK_DECLARE_FUNCTION(vkBindImageMemory);
