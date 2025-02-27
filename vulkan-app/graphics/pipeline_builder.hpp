#pragma once

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class pipeline_builder
{
public:
    pipeline_builder(graphics::context& context);
    ~pipeline_builder();
private:
    graphics::context& m_context;
};

}
