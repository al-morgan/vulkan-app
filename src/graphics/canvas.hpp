#pragma once

#include <array>
#include <vector>

#include <Windows.h>
#include <vulkan/vulkan.h>

#include "graphics.hpp"

namespace graphics
{

struct queue
{
    VkQueue     handle;
    uint32_t    family_index;
};

class canvas
{
    struct framebuffer
    {
        VkImage     image;
        VkImageView view;
        uint32_t    index;
    };

public:
    uint32_t                                m_width;
    uint32_t                                m_height;
    VkInstance                              m_instance;
    VkPhysicalDevice                        m_physical_device;
    VkPhysicalDeviceMemoryProperties        m_memory_properties;
    uint32_t                                m_memory_type_device_local;
    uint32_t                                m_memory_type_host_coherent;
    VkSurfaceKHR                            m_surface;
    std::vector<VkQueueFamilyProperties>    m_queue_family_properties;

    VkDevice                    m_device;
    VkSwapchainKHR              m_swapchain;
    std::vector<framebuffer>    m_framebuffers;
    framebuffer                 m_current_framebuffer;

    std::vector<VkDeviceMemory> allocated_device_memory;
    graphics::queue             graphics_queue;

    canvas(HWND window_handle, uint32_t width, uint32_t height);
    canvas() = delete;
    canvas(graphics::canvas&&) = delete;
    ~canvas();

    void begin_command_buffer(VkCommandBuffer command_buffer);
    void begin_rendering(VkCommandBuffer command_buffer, VkImageView view, VkImageView depth_view);
    void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void submit(VkCommandBuffer command_buffer, VkPipelineStageFlags wait_stage);
    void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);

    void get_next_framebuffer();
    VkImageView image_view() { return m_current_framebuffer.view; };
    void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
    void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
    void present();
    uint32_t get_width();
    uint32_t get_height();
    void begin_frame();

    operator VkDevice() const { return m_device; }

private:

    struct frame
    {
        VkFence     in_flight_fence;
        VkSemaphore render_finished_semaphore;
        VkSemaphore swapchain_semaphore;
    };

    std::array<graphics::canvas::frame, graphics::NUM_FRAMES> m_frames;
    uint32_t m_current_frame = 0;

    void create_swapchain();
};

}
