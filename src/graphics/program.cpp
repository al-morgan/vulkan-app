#include "graphics/program.hpp"

namespace graphics
{

graphics::program::program(VkPipeline pipeline, VkPipelineLayout layout) :
    m_pipeline(pipeline),
    m_layout(layout)
{
}

}