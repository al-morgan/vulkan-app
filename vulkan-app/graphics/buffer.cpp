#include <stdexcept>

#include <vulkan/vulkan.h>
#include "graphics/context.hpp"
#include "graphics/buffer.hpp"
#include "graphics/graphics.hpp"

VkBuffer graphics::buffer::handle()
{
	return m_destination;
}

void* graphics::buffer::data()
{
	return m_mapped_memory;
}

VkDeviceSize graphics::buffer::size()
{
	return m_size;
}

graphics::buffer::buffer(const graphics::context& context, size_t size, VkBufferUsageFlags usage) : m_context(context)
{
	m_size = size;

	// Create host buffer
	VkBufferCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(context.device, &create_info, nullptr, &m_source);

	// Create device buffer
	create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vkCreateBuffer(context.device, &create_info, nullptr, &m_destination);

	// Allocate host memory
	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = size;
	memory_allocate_info.memoryTypeIndex = context.memory_type_host_coherent;
	graphics::check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &m_source_memory));
	graphics::check(vkBindBufferMemory(context.device, m_source, m_source_memory, 0));

	// Allocate device memory
	memory_allocate_info.memoryTypeIndex = context.memory_type_device_local;
	check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &m_destination_memory));
	check(vkBindBufferMemory(context.device, m_destination, m_destination_memory, 0));

	// Map the host memory so we can read it
	check(vkMapMemory(context.device, m_source_memory, 0, size, 0, &m_mapped_memory));
}

graphics::buffer::~buffer()
{
	vkFreeMemory(m_context.device, m_destination_memory, nullptr);
	vkFreeMemory(m_context.device, m_source_memory, nullptr);
	vkDestroyBuffer(m_context.device, m_source, nullptr);
	vkDestroyBuffer(m_context.device, m_destination, nullptr);
}

void graphics::buffer::copy(VkCommandBuffer command_buffer)
{
	VkBufferCopy buffer_copy{};
	buffer_copy.size = m_size;
	vkCmdCopyBuffer(command_buffer, m_source, m_destination, 1, &buffer_copy);
}
