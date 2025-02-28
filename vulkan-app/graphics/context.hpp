#pragma once

#include <vector>

#include <Windows.h>
#include <vulkan/vulkan.h>

namespace graphics
{

struct queue
{
public:
    VkQueue handle;
    uint32_t family_index;
};

class canvas
{
public:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;

    VkSwapchainKHR swapchain;
    VkSemaphore get_next_framebuffer_semaphore;

    uint32_t memory_type_device_local;
    uint32_t memory_type_host_coherent;

    std::vector<VkDeviceMemory> allocated_device_memory;

    graphics::queue transfer_queue;
    graphics::queue graphics_queue;
    graphics::queue present_queue;

    canvas(HWND window_handle, uint32_t width, uint32_t height);
    canvas() = delete;
    canvas(graphics::canvas&&) = delete;
    ~canvas();

    void begin_command_buffer(VkCommandBuffer command_buffer);
    void begin_rendering(VkCommandBuffer command_buffer, VkImageView view, VkImageView depth_view);
    void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void submit(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_stage, VkSemaphore signal_semaphore, VkFence fence);
    void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);
private:
    void create_instance();
    void get_physical_device();
    void create_surface(HWND window_handle);
    void create_device();
};

}
