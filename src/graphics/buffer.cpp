#include <stdexcept>

#include <vulkan/vulkan.h>
#include "graphics/canvas.hpp"
#include "graphics/buffer.hpp"
#include "graphics/graphics.hpp"

VkBuffer graphics::buffer::handle()
{
    return m_destination;
}

void* graphics::buffer::data()
{
    return m_mapped_memory;
}

VkDeviceSize graphics::buffer::size()
{
    return m_size;
}

graphics::buffer::buffer(const graphics::canvas& context, size_t size, VkBufferUsageFlags usage) : m_context(context)
{
    m_size = size;

    // Create host buffer
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(context.m_device, &create_info, nullptr, &m_source);

    // Create device buffer
    create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkCreateBuffer(context.m_device, &create_info, nullptr, &m_destination);

    // Allocate host memory
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(context.m_device, m_source, &memory_requirements);
    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = context.m_memory_type_host_coherent;
    graphics::check(vkAllocateMemory(context.m_device, &memory_allocate_info, nullptr, &m_source_memory));
    graphics::check(vkBindBufferMemory(context.m_device, m_source, m_source_memory, 0));

    vkGetBufferMemoryRequirements(context.m_device, m_destination, &memory_requirements);
    memory_allocate_info.allocationSize = memory_requirements.size;

    // Allocate device memory
    memory_allocate_info.memoryTypeIndex = context.m_memory_type_device_local;
    check(vkAllocateMemory(context.m_device, &memory_allocate_info, nullptr, &m_destination_memory));
    check(vkBindBufferMemory(context.m_device, m_destination, m_destination_memory, 0));

    // Map the host memory so we can read it
    check(vkMapMemory(context.m_device, m_source_memory, 0, size, 0, &m_mapped_memory));
}

graphics::buffer::~buffer()
{
    vkFreeMemory(m_context.m_device, m_destination_memory, nullptr);
    vkFreeMemory(m_context.m_device, m_source_memory, nullptr);
    vkDestroyBuffer(m_context.m_device, m_source, nullptr);
    vkDestroyBuffer(m_context.m_device, m_destination, nullptr);
}

void graphics::buffer::copy(VkCommandBuffer command_buffer)
{
    VkBufferCopy buffer_copy{};
    buffer_copy.size = m_size;
    vkCmdCopyBuffer(command_buffer, m_source, m_destination, 1, &buffer_copy);
}
