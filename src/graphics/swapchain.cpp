//#include <vulkan/vulkan.h>
//
//#include "graphics/swapchain.hpp"
//
//namespace graphics
//{
//
//void graphics::swapchain::get_next_framebuffer(VkSemaphore semaphore)
//{
//    uint32_t image_index = 0;
//    vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, semaphore, nullptr, &image_index);
//    m_current_framebuffer = m_framebuffers[image_index];
//}
//
//void graphics::swapchain::prepare_swapchain_for_writing(VkCommandBuffer command_buffer)
//{
//    VkImageMemoryBarrier barrier{};
//    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    barrier.image = m_current_framebuffer.image;
//    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//    barrier.srcAccessMask = VK_ACCESS_NONE;
//    barrier.dstAccessMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    barrier.subresourceRange.baseArrayLayer = 0;
//    barrier.subresourceRange.baseMipLevel = 0;
//    barrier.subresourceRange.layerCount = 1;
//    barrier.subresourceRange.levelCount = 1;
//    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//}
//
//void graphics::swapchain::prepare_swapchain_for_presentation(VkCommandBuffer command_buffer)
//{
//    VkImageMemoryBarrier barrier{};
//    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    barrier.image = m_current_framebuffer.image;
//    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//    barrier.dstAccessMask = VK_ACCESS_NONE;
//    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    barrier.subresourceRange.baseArrayLayer = 0;
//    barrier.subresourceRange.baseMipLevel = 0;
//    barrier.subresourceRange.layerCount = 1;
//    barrier.subresourceRange.levelCount = 1;
//    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//}
//
//void graphics::swapchain::present(VkSemaphore wait_semaphore)
//{
//    VkPresentInfoKHR present_info{};
//    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    present_info.pSwapchains = &m_swapchain;
//    present_info.swapchainCount = 1;
//    present_info.pWaitSemaphores = &wait_semaphore;
//    present_info.waitSemaphoreCount = 1;
//    present_info.pImageIndices = &m_current_framebuffer.index;
//    vkQueuePresentKHR(graphics_queue.handle, &present_info);
//}
//
//}
//
