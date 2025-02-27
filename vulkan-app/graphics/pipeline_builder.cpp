#include <vulkan/vulkan.h>

#include "graphics/pipeline_builder.hpp"

namespace graphics
{

pipeline_builder::pipeline_builder(graphics::context& context) :
    m_context(context)
{

}

pipeline_builder::~pipeline_builder()
{

}

}
