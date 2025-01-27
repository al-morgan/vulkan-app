#include <vulkan/vulkan.h>

#include "graphics/context.hpp"
#include "graphics/swapchain.hpp"

graphics::swapchain::swapchain(graphics::context& context, uint32_t width, uint32_t height, VkSurfaceKHR surface)
	:	m_context(context)
{
	VkExtent2D extent{};
	extent.width = width;
	extent.height = height;

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;
	create_info.minImageCount = 3;
	create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	vkCreateSwapchainKHR(m_context.device, &create_info, nullptr, &m_swapchain);

	uint32_t swapchain_image_count;
	std::vector<VkImage> images;
	vkGetSwapchainImagesKHR(m_context.device, m_swapchain, &swapchain_image_count, nullptr);
	images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(m_context.device, m_swapchain, &swapchain_image_count, images.data());

	m_framebuffers.resize(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		m_framebuffers[i].index = i;
		m_framebuffers[i].image = images[i];

		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = m_framebuffers[i].image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;
		vkCreateImageView(m_context.device, &image_view_create_info, nullptr, &m_framebuffers[i].view);
	}

	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(m_context.device, &semaphore_create_info, nullptr, &m_semaphore);
}

graphics::framebuffer& graphics::swapchain::get_next_framebuffer()
{
	uint32_t image_index;
	vkAcquireNextImageKHR(m_context.device, m_swapchain, UINT64_MAX, m_semaphore, nullptr, &image_index);
	m_current_framebuffer = m_framebuffers[image_index];

	return m_framebuffers[image_index];
}

graphics::swapchain::~swapchain()
{
	vkDestroySemaphore(m_context.device, m_semaphore, nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(m_framebuffers.size()); i++)
	{
		vkDestroyImageView(m_context.device, m_framebuffers[i].view, nullptr);
	}

	vkDestroySwapchainKHR(m_context.device, m_swapchain, nullptr);
}

void graphics::swapchain::prepare_swapchain_for_writing(VkCommandBuffer command_buffer)
{
	m_context.transition_image(
		command_buffer,
		m_current_framebuffer.image,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_ACCESS_NONE,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
}

void graphics::swapchain::prepare_swapchain_for_presentation(VkCommandBuffer command_buffer)
{
	m_context.transition_image(
		command_buffer,
		m_current_framebuffer.image,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_NONE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);
}

void graphics::swapchain::present(VkSemaphore wait_semaphore)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &m_swapchain;
	present_info.swapchainCount = 1;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = 1;
	present_info.pImageIndices = &m_current_framebuffer.index;
	vkQueuePresentKHR(m_context.graphics_queue.handle, &present_info);
}
