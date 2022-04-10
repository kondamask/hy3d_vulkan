#ifndef HY3D_VULKAN_H
#define HY3D_VULKAN_H 1

//------------------------------------------------------------------------

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
#define MAX_OBJECT_TRANSFORMS 1000

#define MAP_BUFFER_TRUE true

#if INDEX_TYPE_U16
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT16
#endif
#if INDEX_TYPE_U32
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

//------------------------------------------------------------------------
#include "core.h"
#include "renderer_platform.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan\vulkan.h>
//------------------------------------------------------------------------

struct vulkan_image
{
	VkImage handle;
	VkDeviceMemory memoryHandle;
	VkImageView view;
	u32 mipLevels;
};

struct vulkan_mesh
{
	u32 nIndices;
	u32 nVertices;
	u32 nInstances;
	u32 pipelineID;
};

struct vulkan_buffer
{
	VkBuffer handle;
	VkDeviceMemory memoryHandle;
	void *data;
	u64 size;
	u64 writeOffset;
};

struct vulkan_ubo
{
	vulkan_buffer buffer;
	u64 paddedSize;
	
	// TODO: Default constructor that automatically fills the paddedSize
};

struct vulkan_cmd_resources
{
	VkCommandBuffer cmdBuffer;
	VkSemaphore imgAvailableSem;
	VkSemaphore frameReadySem;
	VkFence fence;
	//VkFramebuffer framebuffer;
};

enum VULKAN_PIPELINE_ID
{
	PIPELINE_MESH,
	PIPELINE_WIREFRAME,
	PIPELINE_GRID,

	PIPELINES_COUNT
};

struct vulkan_pipeline
{
	VkPipeline handle;
	VkPipelineLayout layout;
};

struct vulkan_render_object
{
	u32 meshID;
	u32 pipelineID;
	u32 transformID;
};

struct vulkan_context
{
	VkPhysicalDeviceProperties gpuProperties;
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
	vulkan_cmd_resources resources[NUM_RESOURCES];
	u32 swapchainImageCount;
	u32 currentResource;

	VkSampler textureSampler;
	
	VkDescriptorSetLayout globalDescSetLayout;
	VkDescriptorPool globalDescPool;
	VkDescriptorSet globalDescriptor[NUM_SWAPCHAIN_IMAGES];
	
	vulkan_ubo cameraUBO;  // uniform
	vulkan_ubo sceneUBO;   // uniform
	vulkan_buffer transformsStorage;

	vulkan_buffer stagingBuffer;
	vulkan_image depthBuffer;
	vulkan_buffer vertexBuffer;
	vulkan_buffer indexBuffer;
	vulkan_buffer gridBuffer;
	u32 gridVertexCount;	

	vulkan_mesh loadedMesh[MAX_MESHES];
	vulkan_pipeline pipeline[PIPELINES_COUNT];
	u32 loadedMeshCount;

	vulkan_image texture;

	bool canRender;

#if VULKAN_VALIDATION_LAYERS_ON
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HMODULE dll;
#endif
};

//------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------
FUNC_RENDERER_INITIALIZE(VulkanInitialize);

//------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------
global_var vulkan_context *vulkanContext;

//------------------------------------------------------------------------
// NOTE: I INCLUDE THIS HERE BECAUSE IT USES THE GLOBAL VULKAN CONTEXT.
#include "vulkan_functions.h"
#include "vulkan_utils.h"
#endif
