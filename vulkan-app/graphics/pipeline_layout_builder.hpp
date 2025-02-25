#pragma once

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class pipeline_layout_builder
{
public:
    pipeline_layout_builder(graphics::context& context);
    ~pipeline_layout_builder();
    void reset();
    void add_descriptor_set(VkDescriptorSet descriptor_set);
    VkPipelineLayout get_result();
private:
    graphics::context& context;
    
};


}