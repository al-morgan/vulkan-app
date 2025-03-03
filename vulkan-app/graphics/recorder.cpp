#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"
#include "graphics/recorder.hpp"

namespace graphics
{

graphics::recorder::recorder(graphics::canvas& canvas) :
    m_canvas(canvas)
{
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = m_canvas.graphics_queue.family_index;
    graphics::check(vkCreateCommandPool(m_canvas.m_device, &create_info, nullptr, &m_command_pool));

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    for (auto& frame : m_frames)
    {
        vkAllocateCommandBuffers(m_canvas.m_device, &alloc_info, &frame.command_buffer);
    }
}

graphics::recorder::~recorder()
{
    vkDestroyCommandPool(m_canvas.m_device, m_command_pool, nullptr);
}

void graphics::recorder::begin_frame()
{
    m_current_frame = (m_current_frame + 1) % m_frames.size();
    vkResetCommandBuffer(m_frames[m_current_frame].command_buffer, 0);
}

VkCommandBuffer graphics::recorder::get_command_buffer()
{
    return m_frames[m_current_frame].command_buffer;
}

}
