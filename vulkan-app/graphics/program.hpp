#pragma once

#include <vulkan/vulkan.h>

namespace graphics
{

class program
{
public:
    program(VkPipeline pipeline, VkPipelineLayout layout);
    operator VkPipeline() { return m_pipeline; }
    operator VkPipelineLayout() { return m_layout; }
private:
    VkPipeline          m_pipeline;
    VkPipelineLayout    m_layout;
};

}
