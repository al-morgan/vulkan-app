#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "graphics/canvas.hpp"

namespace graphics
{

class set_layout_builder
{
public:
    set_layout_builder(graphics::canvas& context);
    ~set_layout_builder();
    void reset();
    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
    VkDescriptorSetLayout get_result();

private:
    graphics::canvas&                           m_context;
    VkDescriptorSetLayout                       m_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding>   m_bindings;
    std::vector<VkDescriptorSetLayout>          m_layouts;
};

}
