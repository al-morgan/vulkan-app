#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class set_builder
{
public:
    set_builder(graphics::context& context);
    ~set_builder();
    void reset();
    void add_set(uint32_t set, VkDescriptorSetLayout layout);
    VkDescriptorSet get_result();
private:
    graphics::context& m_context;
    std::vector<VkDescriptorSetLayout> m_layouts;
    VkDescriptorPool m_pool;
};

}
