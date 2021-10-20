#ifndef HY3D_VULKAN_H
#define HY3D_VULKAN_H 1

#include "hy3d_base.h"
#include "hy3d_engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES

#include "vulkan\vulkan.h"

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define NUM_SWAPCHAIN_IMAGES 2
#define NUM_RESOURCES 4
#define SHADER_CODE_BUFFER_SIZE 4096
#define SURFACE_FORMAT_COLOR_SPACE VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_FORMAT_FORMAT VK_FORMAT_B8G8R8A8_UNORM

// NOTE(heyyod): THIS IS IMPORTANT! INDICES WE USE TO ACCESS THE UBs
// ARE DEPENDING ON THE CURRENT AQUIRED IMAGE IN THE DRAW LOOP
#define NUM_DESCRIPTORS NUM_SWAPCHAIN_IMAGES

#define MAX_MESHES 10

#define MAP_BUFFER_TRUE true

#if INDEX_TYPE_U16
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT16
#endif
#if INDEX_TYPE_U32
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

struct frame_prep_resource
{
    VkCommandBuffer cmdBuffer;
    VkSemaphore imgAvailableSem;
    VkSemaphore frameReadySem;
    VkFence fence;
    VkFramebuffer framebuffer;
};

struct vulkan_buffer
{
    VkBuffer handle;
    VkDeviceMemory memoryHandle;
    void *data;
    u64 size;
};

struct vulkan_image
{
    VkImage handle;
    VkDeviceMemory memoryHandle;
    VkImageView view;
};

struct vulkan_saved_meshes
{
    u32 nVertices;
    u32 nIndices;
};

struct vulkan_engine
{
    VkPhysicalDeviceProperties properties;
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
    
    // NOTE(heyyod): We have a set of resources that we use in a circular way to prepare
    // the next frame. For now the number of resources is the same as the swapchain images.
    // THIS IS IMPORTANT. THE FRAMEBUFFER CREATION DEPENDS ON THIS!!!
    VkExtent2D windowExtent;
    VkSwapchainKHR swapchain;
    VkCommandPool cmdPool;
    VkRenderPass renderPass;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[NUM_SWAPCHAIN_IMAGES];
    frame_prep_resource resources[NUM_RESOURCES];
    u32 swapchainImageCount;
    u32 currentResource;
    
    vulkan_buffer mvp[NUM_DESCRIPTORS]; // NOTE(heyyod): uniform buffer
    VkSampler textureSampler;
    
    VkDescriptorSet descSets[NUM_DESCRIPTORS];
    VkDescriptorSetLayout descSetLayout;
    VkDescriptorPool descPool;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    
    
    vulkan_image depth;
    
    // NOTE(heyyod): cpu accessible
    vulkan_buffer stagingBuffer;
    
    // NOTE(heyyod): in gpu
    vulkan_buffer vertexBuffer;
    vulkan_buffer indexBuffer;
    u64 vertexBufferWriteOffset;
    u64 indexBufferWriteOffset;
    
    vulkan_saved_meshes savedMeshes[MAX_MESHES];
    u32 savedMeshesCount;
    
    vulkan_image texture;
    
    bool canRender;
    
#if VULKAN_VALIDATION_LAYERS_ON
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    
#ifdef VK_USE_PLATFORM_WIN32_KHR
    HMODULE dll;
#endif
};
global_var vulkan_engine vulkan;

global_var char* shaderFiles[2] = {
    "../build/shaders/triangle.vert.spv",
    "../build/shaders/triangle.frag.spv"
};

namespace Vulkan
{
    function bool Win32LoadDLL();
    function bool Win32Initialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name);
    
    function bool LoadGlobalFunctions();
    function bool LoadInstanceFunctions();
    function bool LoadDeviceFunctions();
    
    function bool CreateSwapchain();
    function bool CreatePipeline();
    function bool CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, vulkan_buffer &buffer, bool mapBuffer = false);
    function bool CreateImage(VkImageType type, VkFormat format, VkExtent3D extent, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectMask, vulkan_image &imageOut);
    
    function void ClearFrameBuffers();
    
    function void ClearImage(vulkan_image &img);
    function void ClearSwapchainImages();
    function void ClearPipeline();
    function void ClearBuffer(vulkan_buffer buffer);
    function void Destroy();
    
    
    function bool Draw(update_data *data);
    
    function bool PushStaged(staged_resources &stagedResources);
    function bool LoadShader(char *filepath, VkShaderModule *shaderOut);
    
    function bool FindMemoryProperties(u32 memoryType, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndexOut);
    
    function void CmdChangeImageLayout(VkImage img, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer cmdBuffer);
    
    function frame_prep_resource *GetNextAvailableResource();
    /*
    function void GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc);
    function void GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs);
    */
#if VULKAN_VALIDATION_LAYERS_ON
    function VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
#endif
}

//-
// NOTE: Macros

#if HY3D_DEBUG
#define AssertSuccess(FuncResult) \
if (FuncResult != VK_SUCCESS)     \
{                                 \
DebugPrint(FuncResult);      \
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
VulkanDeclareFunction(vkGetPhysicalDeviceProperties);
VulkanDeclareFunction(vkGetPhysicalDeviceSurfacePresentModesKHR);
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
VulkanDeclareFunction(vkFlushMappedMemoryRanges);
VulkanDeclareFunction(vkCreateDescriptorSetLayout);
VulkanDeclareFunction(vkDestroyDescriptorSetLayout);
VulkanDeclareFunction(vkCreateDescriptorPool);
VulkanDeclareFunction(vkDestroyDescriptorPool);
VulkanDeclareFunction(vkAllocateDescriptorSets);
VulkanDeclareFunction(vkUpdateDescriptorSets);
VulkanDeclareFunction(vkCreateSampler);
VulkanDeclareFunction(vkDestroySampler);

VulkanDeclareFunction(vkCmdPipelineBarrier);
VulkanDeclareFunction(vkCmdClearColorImage);
VulkanDeclareFunction(vkCmdEndRenderPass);
VulkanDeclareFunction(vkCmdBeginRenderPass);
VulkanDeclareFunction(vkCmdBindPipeline);
VulkanDeclareFunction(vkCmdBindVertexBuffers);
VulkanDeclareFunction(vkCmdBindIndexBuffer);
VulkanDeclareFunction(vkCmdDraw);
VulkanDeclareFunction(vkCmdDrawIndexed);
VulkanDeclareFunction(vkCmdSetViewport);
VulkanDeclareFunction(vkCmdSetScissor);
VulkanDeclareFunction(vkCmdCopyBuffer);
VulkanDeclareFunction(vkCmdBindDescriptorSets);
VulkanDeclareFunction(vkCmdCopyBufferToImage);
VulkanDeclareFunction(vkCmdPushConstants);

#endif
