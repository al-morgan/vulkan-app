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
    const graphics::canvas& m_context;
    VkBuffer m_destination;
    VkBuffer m_source;
    VkDeviceSize m_size;
    VkDeviceMemory m_source_memory;
    VkDeviceMemory m_destination_memory;
    void* m_mapped_memory;
public:
    buffer(const graphics::canvas& context, size_t size, VkBufferUsageFlags usage);
    ~buffer();

    void copy(VkCommandBuffer command_buffer);
    VkBuffer handle();
    void* data();
    VkDeviceSize size();
};
}
