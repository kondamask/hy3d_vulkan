#ifndef HY3D_VULKAN_H
#define HY3D_VULKAN_H 1

#include "hy3d_base.h"
#include "hy3d_engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES

#include "vulkan\vulkan.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define NUM_SWAPCHAIN_IMAGES 2
#define SHADER_CODE_BUFFER_SIZE 4096
#define SURFACE_FORMAT_COLOR_SPACE VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_FORMAT_FORMAT VK_FORMAT_B8G8R8A8_UNORM

struct vulkan_engine
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    u32 graphicsQueueFamilyIndex;
    u32 presentQueueFamilyIndex;
    
    VkSemaphore imageAvailableSem;
    VkSemaphore renderFinishedSem;
    VkFence renderFence;
    
    VkExtent2D windowExtent;
    VkSwapchainKHR swapchain;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[NUM_SWAPCHAIN_IMAGES];
    u32 currentImage;
    u32 swapchainImageCount;
    
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffers[NUM_SWAPCHAIN_IMAGES];
    bool recordCmdBuffer[NUM_SWAPCHAIN_IMAGES];
    
    VkFormat depthFormat;
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthMemory;
    
    VkRenderPass renderPass;
    VkFramebuffer framebuffers[NUM_SWAPCHAIN_IMAGES];
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    void *gpuMemory;
    u32 vertexBufferSize;
    
    bool canRender;
    
#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    
#ifdef VK_USE_PLATFORM_WIN32_KHR
    HMODULE dll;
#endif
};
global vulkan_engine vulkan;

global char* shaderFiles[2] = {
    "..\\build\\shaders\\triangle.vert.spv",
    "..\\build\\shaders\\triangle.frag.spv"
};

namespace Vulkan
{
    function bool Win32LoadDLL();
    function bool Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name);
    
    function bool LoadGlobalFunctions();
    function bool LoadInstanceFunctions();
    function bool LoadDeviceFunctions();
    
    function bool CreateSwapchain();
    function bool CreateCommandBuffers();
    function bool CreateFrameBuffers();
    function bool CreatePipeline();
    function bool ResetCommandBuffers();
    function bool Recreate();
    
    function void ClearFrameBuffers();
    function void ClearSwapchainImages();
    function void ClearPipeline();
    function void Destroy();
    
    function bool Update(update_data *data);
    function bool Draw();
    
    function bool LoadShader(char *filepath, VkShaderModule *shaderOut);
    
    function bool FindMemoryProperties(u32 memoryType, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndexOut);
    
    function void GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc);
    function void GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs);
    
    function VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
}

//-
// NOTE: Macros

#if HY3D_DEBUG
#define AssertSuccess(FuncResult) \
if (FuncResult != VK_SUCCESS)     \
{                                 \
DebugPrint(#FuncResult);      \
AssertBreak();                \
}
#else
#define AssertSuccess(FuncResult) \
if (FuncResult != VK_SUCCESS)     \
return false;
#endif

#define VulkanFuncPtr(func) PFN_##func
#define VulkanDeclareFunction(func) function VulkanFuncPtr(func) func
#define VulkanClearColor(r, g, b, a) \
{                                \
r, g, b, a                   \
}
#define VulkanIsValidHandle(obj) obj != VK_NULL_HANDLE

//-
// NOTE: Global Functions
#define VK_GET_INSTANCE_PROC_ADDR(name) PFN_vkVoidFunction name(VkInstance instance, const char *pName)
typedef VK_GET_INSTANCE_PROC_ADDR(vk_get_instance_proc_addr);
VK_GET_INSTANCE_PROC_ADDR(vkGetInstanceProcAddrStub) { return 0; }
function vk_get_instance_proc_addr *vkGetInstanceProcAddr_ = vkGetInstanceProcAddrStub;
#define vkGetInstanceProcAddr vkGetInstanceProcAddr_

VulkanDeclareFunction(vkCreateInstance);
VulkanDeclareFunction(vkEnumerateInstanceLayerProperties);
VulkanDeclareFunction(vkEnumerateInstanceExtensionProperties);

//-
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

//-
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
VulkanDeclareFunction(vkResetCommandPool);
VulkanDeclareFunction(vkDestroyCommandPool);
VulkanDeclareFunction(vkAllocateCommandBuffers);
VulkanDeclareFunction(vkFreeCommandBuffers);
VulkanDeclareFunction(vkBeginCommandBuffer);
VulkanDeclareFunction(vkEndCommandBuffer);
VulkanDeclareFunction(vkResetCommandBuffer);
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
VulkanDeclareFunction(vkResetFences);
VulkanDeclareFunction(vkDestroyFence);
VulkanDeclareFunction(vkWaitForFences);
VulkanDeclareFunction(vkCreateShaderModule);
VulkanDeclareFunction(vkDestroyShaderModule);
VulkanDeclareFunction(vkCreatePipelineLayout);
VulkanDeclareFunction(vkDestroyPipelineLayout);
VulkanDeclareFunction(vkCreateGraphicsPipelines);
VulkanDeclareFunction(vkDestroyPipeline);
VulkanDeclareFunction(vkCreateBuffer);
VulkanDeclareFunction(vkDestroyBuffer);
VulkanDeclareFunction(vkGetBufferMemoryRequirements);
VulkanDeclareFunction(vkBindBufferMemory);
VulkanDeclareFunction(vkMapMemory);
VulkanDeclareFunction(vkUnmapMemory);

VulkanDeclareFunction(vkCmdPipelineBarrier);
VulkanDeclareFunction(vkCmdClearColorImage);
VulkanDeclareFunction(vkCmdEndRenderPass);
VulkanDeclareFunction(vkCmdBeginRenderPass);
VulkanDeclareFunction(vkCmdBindPipeline);
VulkanDeclareFunction(vkCmdBindVertexBuffers);
VulkanDeclareFunction(vkCmdDraw);

#endif
