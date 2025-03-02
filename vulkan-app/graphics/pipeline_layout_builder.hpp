#pragma once

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"

namespace graphics
{

class pipeline_layout_builder
{
public:
    pipeline_layout_builder(graphics::canvas& context);
    ~pipeline_layout_builder();
    void reset();
    void add_set(uint32_t layout, VkDescriptorSetLayout descriptor_set);
    VkPipelineLayout get_result();
private:
    graphics::canvas& m_context;
    std::vector<VkDescriptorSetLayout> m_set_layouts;
    std::vector<VkPipelineLayout> m_layouts;
};


}