#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class pipeline_builder
{
public:
    pipeline_builder(graphics::context& context);
    ~pipeline_builder();
    void reset();
    VkPipeline get_result();
    void set_layout(VkPipelineLayout layout);
    void set_vertex_shader(VkShaderModule shader);
    void set_fragment_shader(VkShaderModule shader);
private:
    graphics::context& m_context;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkShaderModule  m_vertex_shader = VK_NULL_HANDLE;
    VkShaderModule m_fragment_shader = VK_NULL_HANDLE;
    std::vector<VkPipeline> m_pipelines;
};

}
