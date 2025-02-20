#include "graphics/context.hpp"
#include "graphics/pipeline_layout.hpp"

namespace graphics
{

pipeline_layout::pipeline_layout(graphics::context& context) :
    m_context(context)
{

}

void pipeline_layout::add_set(uint32_t set)
{
    m_descriptor_set_layout_ci.bindingCount = 0;
    m_descriptor_set_layout_ci.flags = 0;
    m_descriptor_set_layout_ci.
}

}