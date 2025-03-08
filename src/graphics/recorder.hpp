#pragma once

#include <array>

#include <vulkan/vulkan.h>

#include "graphics/graphics.hpp"
#include "graphics/canvas.hpp"
#include "graphics/program.hpp"

namespace graphics
{

class recorder
{
public:
    recorder(graphics::canvas& canvas);
    ~recorder();
    recorder(graphics::recorder&) = delete;

    void begin_frame();
    void begin_rendering(uint32_t width, uint32_t height, VkImageView color_view, VkImageView depth_view);
    void use_program(graphics::program& program);
    void end_rendering();

    operator VkCommandBuffer() { return m_frames[m_frame_index].command_buffer; }

    VkCommandBuffer get_command_buffer();

    void end_frame();

private:
    struct frame
    {
        VkCommandBuffer command_buffer;
    };

    const graphics::canvas&                 m_canvas;
    VkCommandPool                           m_command_pool;
    std::vector<graphics::recorder::frame>  m_frames;
    uint32_t                                m_frame_index = 0;
};

}
