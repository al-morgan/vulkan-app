#pragma once

#include <vector>

#include <Windows.h>
#include <vulkan/vulkan.h>

namespace graphics
{

struct queue
{
    VkQueue     handle;
    uint32_t    family_index;
};

struct framebuffer
{
    VkImage image;
    VkImageView view;
    uint32_t index;
};

class canvas
{
public:
    VkInstance                  m_instance;
    VkSurfaceKHR                m_surface;
    VkPhysicalDevice            m_physical_device;
    VkDevice                    m_device;
    VkSwapchainKHR              m_swapchain;

    VkSemaphore                 m_semaphore;
    std::vector<graphics::framebuffer>    m_framebuffers;
    graphics::framebuffer       m_current_framebuffer;

    uint32_t memory_type_device_local;
    uint32_t memory_type_host_coherent;

    std::vector<VkDeviceMemory> allocated_device_memory;

    graphics::queue graphics_queue;

    canvas(HWND window_handle, uint32_t width, uint32_t height);
    canvas() = delete;
    canvas(graphics::canvas&&) = delete;
    ~canvas();

    void begin_command_buffer(VkCommandBuffer command_buffer);
    void begin_rendering(VkCommandBuffer command_buffer, VkImageView view, VkImageView depth_view);
    void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void submit(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_stage, VkSemaphore signal_semaphore, VkFence fence);
    void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);

    void get_next_framebuffer(VkSemaphore semaphore);
    VkSemaphore get_semaphore() { return m_semaphore; }
    VkImageView image_view() { return m_current_framebuffer.view; };
    void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
    void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
    void present(VkSemaphore wait_semaphore);


private:
    void create_instance();
    void get_physical_device();
    void create_surface(HWND window_handle);
    void create_device();
};

}
