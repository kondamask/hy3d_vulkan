#ifndef HY3D_VULKAN_H
#define HY3D_VULKAN_H 1

#include "hy3d_base.h"
#include "hy3d_engine.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan\vulkan.h"

#define NUM_SWAPCHAIN_IMAGES 2
#define NUM_RESOURCES 4
#define SHADER_CODE_BUFFER_SIZE 4096
#define SURFACE_FORMAT_COLOR_SPACE VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_FORMAT_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define DEPTH_BUFFER_FORMAT VK_FORMAT_D32_SFLOAT

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
    //VkFramebuffer framebuffer;
};

struct frame_data
{
    
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
    u32 mipLevels;
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
    
    VkSampleCountFlagBits msaaSamples;
    vulkan_image msaa;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    u32 graphicsQueueFamilyIndex;
    u32 presentQueueFamilyIndex;
    u32 transferQueueFamilyIndex;
    
    // NOTE(heyyod): We have a set of resources that we use in a circular way to prepare
    // the next frame. For now the number of resources is the same as the swapchain images.
    // THIS IS IMPORTANT. THE FRAMEBUFFER CREATION DEPENDS ON THIS!!!
    VkExtent2D windowExtent;
    VkSwapchainKHR swapchain;
    VkCommandPool cmdPool;
    VkRenderPass renderPass;
    VkImage swapchainImages[NUM_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[NUM_SWAPCHAIN_IMAGES];
    VkFramebuffer framebuffers[NUM_SWAPCHAIN_IMAGES];
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
#include "hy3d_vulkan_functions.h"

global_var char* shaderFiles[2] = {
    "../build/shaders/triangle.vert.spv",
    "../build/shaders/triangle.frag.spv"
};

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

#define VulkanClearColor(r, g, b, a) \
{                                \
r, g, b, a                   \
}
#define VulkanIsValidHandle(obj) obj != VK_NULL_HANDLE

#endif
