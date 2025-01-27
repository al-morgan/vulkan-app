#pragma once

#include <vulkan/vulkan.h>

#include "graphics/graphics.hpp"
#include "graphics/context.hpp"

namespace graphics
{
	class swapchain
	{
	public:
		swapchain(graphics::context& context, uint32_t width, uint32_t height, VkSurfaceKHR surface);
		swapchain() = delete;
		swapchain(graphics::swapchain&&) = delete;
		~swapchain();
		void get_next_framebuffer();
		VkSemaphore get_semaphore() { return m_semaphore; }
		VkImageView image_view() { return m_current_framebuffer.view; };
		void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
		void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
		void present(VkSemaphore wait_semaphore);
	private:
		VkSwapchainKHR m_swapchain;
		VkSemaphore m_semaphore;
		const graphics::context& m_context;
		std::vector<framebuffer> m_framebuffers;
		graphics::framebuffer m_current_framebuffer;
	};
}
