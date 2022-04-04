#ifndef INCLUDE_VULKAN_UTILS
#define INCLUDE_VULKAN_UTILS

#include "core.h"

static_func u64 VulkanGetUniformBufferPaddedSize(u64 originalSize)
{
	u64 minUboAlignment = vulkan.gpuProperties.limits.minUniformBufferOffsetAlignment;
	u64 alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

static_func bool VulkanFindMemoryProperties(u32 reqMemType, VkMemoryPropertyFlags reqMemProperties, u32 &memoryIndexOut)
{
	// NOTE(heyyod): We assume we have already set the memory properties during creation.
	u32 memoryCount = vulkan.memoryProperties.memoryTypeCount;
	for (memoryIndexOut = 0; memoryIndexOut < memoryCount; ++memoryIndexOut)
	{
		uint32_t memoryType = (1 << memoryIndexOut);
		bool isRequiredMemoryType = reqMemType & memoryType;
		if (isRequiredMemoryType)
		{
			VkMemoryPropertyFlags properties = vulkan.memoryProperties.memoryTypes[memoryIndexOut].propertyFlags;
			bool hasRequiredProperties = ((properties & reqMemProperties) == reqMemProperties);
			if (hasRequiredProperties)
				return true;
		}
	}
	return false;
}

#if HY3D_DEBUG
#define VK_CHECK_RESULT(call)	\
	{	\
	VkResult _result = call;	\
	if (_result != VK_SUCCESS)	\
	{	\
	DebugPrint("VULKAN CALL FAILED: " << __FILE__ << " at line " << __LINE__ << ": " << #call); \
	return false; \
	}	\
	}
#else
#define VK_CHECK_RESULT(call) call
#endif

#define VK_CHECK_HANDLE(handle) (handle != VK_NULL_HANDLE)

#endif // INCLUDE_VULKAN_UTILS