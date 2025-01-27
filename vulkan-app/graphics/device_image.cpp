#include <stdexcept>

#include <vulkan/vulkan.h>
#include "graphics/context.hpp"
#include "graphics/device_image.hpp"

static void check(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error!");
	}
}

VkImage graphics::device_image::handle()
{
	return m_image;
}

graphics::device_image::device_image(const graphics::context& context, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) : m_context(context)
{
	VkImageCreateInfo create_info{};

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.arrayLayers = 1;
	create_info.extent.height = height;
	create_info.extent.width = width;
	create_info.extent.depth = 1;
	create_info.format = format;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.mipLevels = 1;
	create_info.usage = usage;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	vkCreateImage(context.device, &create_info, nullptr, &m_image);

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(context.device, m_image, &memory_requirements);
	
	// Allocate host memory
	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = context.memory_type_device_local;
	check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &m_memory));

	vkBindImageMemory(context.device, m_image, m_memory, 0);

	VkImageViewCreateInfo iv_create_info{};
	iv_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	iv_create_info.image = m_image;
	iv_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	iv_create_info.format = format;
	iv_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	iv_create_info.subresourceRange.baseArrayLayer = 0;
	iv_create_info.subresourceRange.baseMipLevel = 0;
	iv_create_info.subresourceRange.layerCount = 1;
	iv_create_info.subresourceRange.levelCount = 1;


	//iv_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	vkCreateImageView(context.device, &iv_create_info, nullptr, &m_view);
}

graphics::device_image::~device_image()
{
	vkFreeMemory(m_context.device, m_memory, nullptr);
	vkDestroyImageView(m_context.device, m_view, nullptr);
	vkDestroyImage(m_context.device, m_image, nullptr);
}
