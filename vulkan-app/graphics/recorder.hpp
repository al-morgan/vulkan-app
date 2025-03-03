#pragma once

#include <array>

#include <vulkan/vulkan.h>

#include "graphics/graphics.hpp"
#include "graphics/canvas.hpp"

namespace graphics
{

class recorder
{
public:
    recorder(graphics::canvas& canvas);
    ~recorder();

    recorder(graphics::recorder&) = delete;

    void begin_frame();
    VkCommandBuffer get_command_buffer();

private:

    struct frame
    {
        VkCommandBuffer command_buffer;
    };

    graphics::canvas&   m_canvas;
    VkCommandPool       m_command_pool;

    std::array<graphics::recorder::frame, graphics::NUM_FRAMES> m_frames{};
    uint32_t m_current_frame = 0;
};

}