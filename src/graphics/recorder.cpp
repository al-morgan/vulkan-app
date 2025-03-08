#include "recorder.hpp"
#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"
#include "graphics/recorder.hpp"
#include "graphics/program.hpp"

constexpr uint32_t num_frames = 2;

namespace graphics
{

graphics::recorder::recorder(graphics::canvas& canvas) :
    m_canvas(canvas)
{
    // Create command pool
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = m_canvas.graphics_queue.family_index;
    graphics::check(vkCreateCommandPool(m_canvas, &create_info,
        nullptr, &m_command_pool));

    // Create command buffers for each frame
    m_frames.resize(num_frames);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    for (auto& frame : m_frames)
    {
        graphics::check(vkAllocateCommandBuffers(m_canvas.m_device,
            &alloc_info, &frame.command_buffer));
    }
}

graphics::recorder::~recorder()
{
    vkDestroyCommandPool(m_canvas.m_device, m_command_pool, nullptr);
}

void graphics::recorder::begin_frame()
{
    m_frame_index = (m_frame_index + 1) % m_frames.size();
    vkResetCommandBuffer(m_frames[m_frame_index].command_buffer, 0);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(m_frames[m_frame_index].command_buffer, &begin_info);
}

void graphics::recorder::begin_rendering(uint32_t width, uint32_t height, VkImageView color_view, VkImageView depth_view)
{
    VkClearValue clear_value{};

    VkRect2D render_area{};
    render_area.extent.width = width;
    render_area.extent.height = height;

    VkRenderingAttachmentInfo color_attachment_info{};
    color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment_info.clearValue = clear_value;
    color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    color_attachment_info.imageView = color_view;
    color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    clear_value.depthStencil.depth = 1.0f;
    VkRenderingAttachmentInfo depth_attachment_info{};
    depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment_info.clearValue = clear_value;
    depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_info.imageView = depth_view;

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pColorAttachments = &color_attachment_info;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.layerCount = 1;
    rendering_info.renderArea = render_area;
    rendering_info.pDepthAttachment = &depth_attachment_info;

    vkCmdBeginRendering(m_frames[m_frame_index].command_buffer, &rendering_info);
}

void graphics::recorder::use_program(graphics::program& program)
{
    vkCmdBindPipeline(m_frames[m_frame_index].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program);
}

void graphics::recorder::end_rendering()
{
    vkCmdEndRendering(m_frames[m_frame_index].command_buffer);
}

VkCommandBuffer graphics::recorder::get_command_buffer()
{
    return m_frames[m_frame_index].command_buffer;
}

void graphics::recorder::end_frame()
{
    vkEndCommandBuffer(m_frames[m_frame_index].command_buffer);
}

}   // namespace
