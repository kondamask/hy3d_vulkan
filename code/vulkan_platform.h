#ifndef HY3D_VULKAN_H
#define HY3D_VULKAN_H 1

#include "core.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan\vulkan.h>

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
#define MAX_RENDER_OBJECTS 1000

#define MAP_BUFFER_TRUE true

#if INDEX_TYPE_U16
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT16
#endif
#if INDEX_TYPE_U32
#define VULKAN_INDEX_TYPE VK_INDEX_TYPE_UINT32
#endif

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
	vulkan_buffer cameraBuffer;  // uniform
	vulkan_buffer sceneBuffer;   // dynamic uniform
	VkDescriptorSet globalDescriptor;
};

enum VulkanPipelineTypes
{
	PIPELINE_MESH,

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

global_var struct
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
	frame_prep_resource resources[NUM_RESOURCES];
	u32 swapchainImageCount;
	u32 currentResource;

	frame_data frameData[NUM_SWAPCHAIN_IMAGES];
	VkSampler textureSampler;

	VkDescriptorSetLayout globalDescSetLayout;
	VkDescriptorPool globalDescPool;

	// NOTE(heyyod): in gpu
	vulkan_buffer stagingBuffer;
	vulkan_image depthBuffer;
	vulkan_buffer vertexBuffer;
	vulkan_buffer indexBuffer;

	vulkan_mesh loadedMesh[MAX_MESHES];
	vulkan_pipeline pipeline[PIPELINES_COUNT];
	vulkan_buffer staticTransformsBuffer;
	u32 loadedMeshCount;

	vulkan_image texture;

	bool canRender;

#if VULKAN_VALIDATION_LAYERS_ON
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	HMODULE dll;
#endif
} vulkan;

#include "vulkan_functions.h"

global_var char* shaderFiles[2] = {
	"../build/shaders/triangle.vert.spv",
	"../build/shaders/triangle.frag.spv"
};

//-
// NOTE: Macros

#if HY3D_DEBUG
#define AssertSuccess(FuncResult) 	\
	if (FuncResult != VK_SUCCESS)	\
	{ \
	DebugPrint(FuncResult); \
	AssertBreak(); \
	}
#else
#define AssertSuccess(FuncResult)	\
	if (FuncResult != VK_SUCCESS)	\
	return false;
#endif


#define VulkanIsValidHandle(obj) obj != VK_NULL_HANDLE

	/***********************************
 * FUNCTIONS
 **********************************/
	static_func bool VulkanLoadCode();
	static_func bool VulkanInitialize(HINSTANCE &wndInstance, HWND &wndHandle, const char *name);
	static_func void VulkanDestroy();
	
	static_func bool VulkanCreateRenderPass();
	static_func void VulkanClearRenderPass();
	
	static_func bool VulkanCreateFrameBuffers();
	static_func void VulkanClearFrameBuffers();
	
	static_func bool VulkanCreateDepthBuffer();
	
	static_func bool VulkanCreateMSAABuffer();
	
	static_func bool VulkanCreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, vulkan_buffer &buffer, bool mapBuffer = false);
	static_func void VulkanClearBuffer(vulkan_buffer buffer);
	
	static_func bool VulkanCreateSwapchain();
	static_func void VulkanClearSwapchain();
	static_func void VulkanClearSwapchainImages();
	
	static_func bool VulkanCreateImage(VkImageType type, VkFormat format, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImageAspectFlags aspectMask, vulkan_image &imageOut);
	static_func void VulkanClearImage(vulkan_image &img);

	static_func bool VulkanCreatePipeline();
	static_func void VulkanClearPipeline();

	
	struct update_data;
	static_func bool VulkanDraw(update_data *data);

	static_func bool VulkanPushStaged(staged_resources &stagedResources);
	static_func bool VulkanLoadShader(char *filepath, VkShaderModule *shaderOut);

	static_func void VulkanPickMSAA(MSAA_OPTIONS msaa);
	static_func bool VulkanFindMemoryProperties(u32 memoryType, VkMemoryPropertyFlags requiredProperties, u32 &memoryIndexOut);
	static_func u64 VulkanGetUniformBufferPaddedSize(u64 originalSize);

	static_func bool VulkanChangeGraphicsSettings(graphics_settings settings, CHANGE_GRAPHICS_SETTINGS newSettings);
	static_func void VulkanCmdChangeImageLayout(VkCommandBuffer cmdBuffer, VkImage imgHandle, image *imageInfo, VkImageLayout oldLayout, VkImageLayout newLayout);

	static_func frame_prep_resource *VulkanGetNextAvailableResource();

	//static_func void GetVertexBindingDesc(vertex2 &v, VkVertexInputBindingDescription &bindingDesc);
	//static_func void GetVertexAttributeDesc(vertex2 &v, VkVertexInputAttributeDescription *attributeDescs);

#endif
