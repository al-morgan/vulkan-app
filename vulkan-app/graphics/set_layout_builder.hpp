#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/context.hpp"

namespace graphics
{

class set_layout_builder
{
public:
    set_layout_builder(graphics::context& context);
    ~set_layout_builder();
    void reset();
    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
    VkDescriptorSetLayout get_result();

private:
    graphics::context&                          m_context;
    VkDescriptorSetLayout                       m_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding>   m_bindings;
    std::vector<VkDescriptorSetLayout>          m_layouts;
    VkDescriptorPool                            m_descriptor_pool;
};

}
