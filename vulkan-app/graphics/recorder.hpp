#pragma once

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"

namespace graphics
{

class recorder
{
public:
    recorder(graphics::canvas& canvas);

private:
    graphics::canvas& m_canvas;
};

}