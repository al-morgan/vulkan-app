#include <stdexcept>

#include <vulkan/vulkan.h>
#include "graphics/canvas.hpp"
#include "graphics/image.hpp"

static void check(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error!");
	}
}

graphics::image::image(const graphics::canvas& context, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool host_memory) : m_context(context), m_aspect(aspect)
{
	VkImageCreateInfo image_ci{};
	image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_ci.arrayLayers = 1;
	image_ci.extent.height = height;
	image_ci.extent.width = width;
	image_ci.extent.depth = 1;
	image_ci.format = format;
	image_ci.imageType = VK_IMAGE_TYPE_2D;
	image_ci.mipLevels = 1;
	image_ci.usage = usage;
	image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
	check(vkCreateImage(context.m_device, &image_ci, nullptr, &m_destination));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(context.m_device, m_destination, &memory_requirements);

	// Allocate device memory
	VkMemoryAllocateInfo memory_ai{};
	memory_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_ai.allocationSize = memory_requirements.size;
	memory_ai.memoryTypeIndex = context.m_memory_type_device_local;
	check(vkAllocateMemory(context.m_device, &memory_ai, nullptr, &m_destination_memory));

	// Bind device memory
	vkBindImageMemory(context.m_device, m_destination, m_destination_memory, 0);

	// Create image view.
	// TODO: Maybe only do this on demand?
	VkImageViewCreateInfo image_view_ci{};
	image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_ci.image = m_destination;
	image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_ci.format = format;
	image_view_ci.subresourceRange.aspectMask = m_aspect;
	image_view_ci.subresourceRange.baseArrayLayer = 0;
	image_view_ci.subresourceRange.baseMipLevel = 0;
	image_view_ci.subresourceRange.layerCount = 1;
	image_view_ci.subresourceRange.levelCount = 1;
	vkCreateImageView(context.m_device, &image_view_ci, nullptr, &m_view);

	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	if (host_memory)
	{
		// Create host buffer
		VkBufferCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.size = memory_requirements.size;
		create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vkCreateBuffer(context.m_device, &create_info, nullptr, &m_source);

		// Allocate host memory
		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = context.m_memory_type_host_coherent;
		check(vkAllocateMemory(context.m_device, &memory_allocate_info, nullptr, &m_source_memory));
		check(vkBindBufferMemory(context.m_device, m_source, m_source_memory, 0));

		// Map the host memory so we can read it
		check(vkMapMemory(context.m_device, m_source_memory, 0, memory_requirements.size, 0, &m_mapped_memory));
	}

}

graphics::image::~image()
{
	close();
	vkFreeMemory(m_context.m_device, m_destination_memory, nullptr);
	vkDestroyImageView(m_context.m_device, m_view, nullptr);
	vkDestroyImage(m_context.m_device, m_destination, nullptr);
}

void graphics::image::send(VkCommandBuffer command_buffer)
{
	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask = m_aspect;
	region.imageSubresource.layerCount = 1;

	vkCmdCopyBufferToImage(command_buffer, m_source, m_destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// TODO: need to transition to final layout
}


void graphics::image::transition(VkCommandBuffer command_buffer, VkImageLayout layout, VkPipelineStageFlags source_stage_mask, VkAccessFlags source_access_mask, VkPipelineStageFlags destination_stage_mask, VkAccessFlags destination_access_mask)
{
	if (layout == m_layout)
	{
		return;
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = m_destination;
	barrier.oldLayout = m_layout;
	barrier.newLayout = layout;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.subresourceRange.aspectMask = m_aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	vkCmdPipelineBarrier(command_buffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}


void graphics::image::close()
{
	if (m_source_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_context.m_device, m_source_memory, nullptr);
		vkDestroyBuffer(m_context.m_device, m_source, nullptr);
		m_source_memory = VK_NULL_HANDLE;
		m_source = VK_NULL_HANDLE;
	}
}
