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
public:

    // Device

    uint32_t                                m_width;
    uint32_t                                m_height;
    VkInstance                              m_instance;
    VkPhysicalDevice                        m_physical_device;
    VkPhysicalDeviceMemoryProperties        m_memory_properties;
    uint32_t                                m_memory_type_device_local;
    uint32_t                                m_memory_type_host_coherent;
    VkSurfaceKHR                            m_surface;
    std::vector<VkQueueFamilyProperties>    m_queue_family_properties;
    VkDevice                                m_device;

    // Swapchain

    typedef struct
    {
        VkImage image;
        VkImageView image_view;
    } framebuffer;

    VkSwapchainKHR                              m_swapchain;
    std::vector<graphics::canvas::framebuffer>  m_framebuffers;
    uint32_t                                    m_framebuffer_index = 0;

    // Frame

    typedef struct
    {
        VkFence     in_flight_fence;
        VkSemaphore render_finished_semaphore;
        VkSemaphore swapchain_semaphore;
    } frame;

    std::vector<graphics::canvas::frame>    m_frames;

    uint32_t                                m_frame_index = 0;

    std::vector<VkDeviceMemory> allocated_device_memory;
    
    graphics::queue             graphics_queue;

    canvas(HWND window_handle, uint32_t width, uint32_t height);
    canvas() = delete;
    canvas(graphics::canvas&&) = delete;
    ~canvas();

    void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void submit(VkCommandBuffer command_buffer, VkPipelineStageFlags wait_stage);
    void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);

    VkImageView image_view() { return m_framebuffers[m_framebuffer_index].image_view; };
    void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
    void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
    void present();
    uint32_t get_width();
    uint32_t get_height();
    void begin_frame();

    operator VkDevice() const { return m_device; }

private:

    //void create_swapchain();
};

}
