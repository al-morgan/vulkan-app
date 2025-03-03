#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"
#include "graphics/recorder.hpp"

namespace graphics
{

graphics::recorder::recorder(graphics::canvas& canvas) :
    m_canvas(canvas)
{

}

}