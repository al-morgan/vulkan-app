#pragma once

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"

namespace graphics
{

class descriptor_set_builder
{
public:
    descriptor_set_builder(graphics::canvas& context);
    ~descriptor_set_builder();
    VkDescriptorSet get_result();
    void set_layout(VkDescriptorSetLayout layout);
private:
    graphics::canvas& m_context;
    VkDescriptorSetLayout m_layout;
    VkDescriptorPool m_descriptor_pool;

};


}