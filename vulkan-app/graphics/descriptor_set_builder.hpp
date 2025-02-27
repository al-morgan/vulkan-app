#pragma once

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class descriptor_set_builder
{
public:
    descriptor_set_builder(graphics::context& context);
    ~descriptor_set_builder();
private:
    graphics::context& m_context;
    VkDescriptorPool m_descriptor_pool;

};


}