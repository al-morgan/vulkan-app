#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphics/canvas.hpp"
#include "window.hpp"

namespace app
{

class engine
{
private:
    graphics::canvas& context;
    VkFence m_in_flight_fence;
    VkSemaphore m_render_finished_semaphore;

public:
    engine(graphics::canvas& context);
    ~engine();
    void update(graphics::canvas& context, app::window& window);
};
}
