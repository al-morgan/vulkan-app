#pragma once

#include <vulkan/vulkan.h>
#include "graphics/context.hpp"

namespace graphics
{
	/// <summary>
	/// Handles transfering buffer data to the GPU
	/// </summary>
	class image
	{
	private:
		const graphics::canvas& m_context;
		VkImage m_destination = VK_NULL_HANDLE;
		VkBuffer m_source = VK_NULL_HANDLE;
		VkDeviceMemory m_source_memory = VK_NULL_HANDLE;
		VkDeviceMemory m_destination_memory = VK_NULL_HANDLE;
		void* m_mapped_memory = nullptr;
		VkImageView m_view = VK_NULL_HANDLE;
		VkImageAspectFlags m_aspect = 0;
		VkImageLayout m_layout;
	public:
		image(const graphics::canvas& context, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, bool host_memory);
		~image();

		/// <summary>
		/// Returns the device-local buffer handle.
		/// </summary>
		/// <returns>The buffer handle.</returns>
		VkImage handle() { return m_destination; }

		/// <summary>
		/// Returns a pointer to the host-local data.
		/// </summary>
		/// <returns>A pointer to the host-local data.</returns>
		void* data() { return m_mapped_memory; };

		/// <summary>
		/// Returns the image view.
		/// </summary>
		/// <returns>The view for this image.</returns>
		VkImageView view() { return m_view; }

		/// <summary>
		/// Sends buffer data to the GPU.
		/// </summary>
		/// <param name="command_buffer">The active command buffer.</param>
		void send(VkCommandBuffer command_buffer);

		/// <summary>
		/// Transition the image to a new layout.
		/// </summary>
		/// <param name="command_buffer"></param>
		/// <param name="layout"></param>
		void transition(VkCommandBuffer command_buffer, VkImageLayout layout, VkPipelineStageFlags source_stage_mask, VkAccessFlags source_access_mask, VkPipelineStageFlags destination_stage_mask, VkAccessFlags destination_access_mask);

		/// <summary>
		/// Closes the connection between the local buffer and the image on
		/// the GPU, freeing up resources.
		/// </summary>
		void close();
	};
}
