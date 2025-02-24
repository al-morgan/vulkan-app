#pragma once

#include <vulkan/vulkan.h>
#include "graphics/context.hpp"

namespace graphics
{
	/// <summary>
	/// Handles transfering buffer data to the GPU
	/// </summary>
	class buffer
	{
	private:
		const graphics::context& m_context;
		VkBuffer m_destination;
		VkBuffer m_source;
		VkDeviceSize m_size;
		VkDeviceMemory m_source_memory;
		VkDeviceMemory m_destination_memory;
		void* m_mapped_memory;
	public:
		buffer(const graphics::context& context, size_t size, VkBufferUsageFlags usage);
		~buffer();
		
		/// <summary>
		/// Copy the buffer data to the GPU. Call from a command buffer.
		/// </summary>
		/// <param name="command_buffer">The current command buffer</param>
		void copy(VkCommandBuffer command_buffer);

		/// <summary>
		/// Returns the device-local buffer handle.
		/// </summary>
		/// <returns>The buffer handle.</returns>
		VkBuffer handle();
		
		/// <summary>
		/// Returns a pointer to the host-local data.
		/// </summary>
		/// <returns>A pointer to the host-local data.</returns>
		void* data();

		/// <summary>
		/// Returns the size of the buffer.
		/// </summary>
		/// <returns>The size of the buffer, in bytes.</returns>
		VkDeviceSize size();
	};
}