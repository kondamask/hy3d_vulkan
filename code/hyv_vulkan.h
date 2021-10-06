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

    u32 queueFamilyIndex;
    VkQueue queue;
    //VkQueue graphicsQueue;
    //VkQueue presentQueue;

    VkSwapchainKHR swapchain;
    u32 swapchainImageCount;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView imageViews[NUM_SWAPCHAIN_IMAGES];

    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffers[NUM_SWAPCHAIN_IMAGES];

    VkFormat depthFormat;
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthMemory;

    HMODULE dll;

#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

//=====================================================================
// NOTE: Macros
#define VulkanFuncPtr(func) PFN_##func
#define VulkanDeclareFunction(func) VulkanFuncPtr(func) func

//=====================================================================
// NOTE: Global Functions
#define VK_GET_INSTANCE_PROC_ADDR(name) PFN_vkVoidFunction name(VkInstance instance, const char *pName)
typedef VK_GET_INSTANCE_PROC_ADDR(vk_get_instance_proc_addr);
VK_GET_INSTANCE_PROC_ADDR(vkGetInstanceProcAddrStub) { return 0; }
static vk_get_instance_proc_addr *vkGetInstanceProcAddr_ = vkGetInstanceProcAddrStub;
#define vkGetInstanceProcAddr vkGetInstanceProcAddr_

VulkanDeclareFunction(vkCreateInstance);
VulkanDeclareFunction(vkEnumerateInstanceLayerProperties);

//=====================================================================
// NOTE: Instance Functions
VulkanDeclareFunction(vkDestroyInstance);

VulkanDeclareFunction(vkCreateDebugUtilsMessengerEXT);
VulkanDeclareFunction(vkDestroyDebugUtilsMessengerEXT);

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

VulkanDeclareFunction(vkCreateCommandPool);
VulkanDeclareFunction(vkDestroyCommandPool);
VulkanDeclareFunction(vkAllocateCommandBuffers);
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

