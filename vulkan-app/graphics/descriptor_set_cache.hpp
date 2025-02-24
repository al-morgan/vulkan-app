#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class descriptor_set_cache
{
public:
    descriptor_set_cache(graphics::context& context);
    void reset();
    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
    void commit();

private:
    graphics::context&                          m_context;
    std::vector<VkDescriptorSet>                m_descriptor_sets;
    VkDescriptorSetLayout                       m_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding>   m_bindings;
    VkDescriptorPool                            m_descriptor_pool;
};

};
