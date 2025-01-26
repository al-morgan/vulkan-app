#pragma once

#include <vulkan/vulkan.h>

namespace transfer
{
	class buffer
	{
	private:
		const gfx::context& m_context;
		VkBuffer m_destination;
		VkBuffer m_source;
		VkDeviceSize m_size;
		VkDeviceMemory m_source_memory;
		VkDeviceMemory m_destination_memory;
		void* m_memory;
	public:
		buffer(const gfx::context& context, size_t size, VkBufferUsageFlags usage);
		~buffer();

		void copy(VkCommandBuffer command_buffer);
		VkBuffer handle();
		void* data();
	};
}