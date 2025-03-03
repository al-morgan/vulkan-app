//#pragma once
//
//#include <vulkan/vulkan.h>
//
//namespace graphics
//{
//
//class swapchain
//{
//public:
//    void get_next_framebuffer(VkSemaphore semaphore);
//    VkImageView image_view() { return m_current_framebuffer.view; };
//    void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
//    void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
//    void present(VkSemaphore wait_semaphore);
//
//private:
//    VkSwapchainKHR              m_swapchain;
//    VkSemaphore                 m_semaphore;
//
//};
//
//}