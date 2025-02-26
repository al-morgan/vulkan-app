#include <vulkan/vulkan.h>

#include "graphics/context.hpp"
#include "graphics/set_layout_builder.hpp"

namespace graphics
{

set_layout_builder::set_layout_builder(graphics::context& context) :
    m_context(context)
{

}

set_layout_builder::~set_layout_builder()
{
    for (auto layout : m_layouts)
    {
        vkDestroyDescriptorSetLayout(m_context.device, layout, nullptr);
    }
}

void set_layout_builder::reset()
{
    m_bindings.clear();
    m_layout = VK_NULL_HANDLE;
}

void set_layout_builder::add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding dsl_binding{};
    dsl_binding.binding = binding;
    dsl_binding.descriptorType = descriptor_type;
    dsl_binding.stageFlags = stage_flags;
    dsl_binding.descriptorCount = 1;
    m_bindings.push_back(dsl_binding);
}

VkDescriptorSetLayout set_layout_builder::get_result()
{
    VkDescriptorSetLayout layout;

    VkDescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.bindingCount = m_bindings.size();
    layout_create_info.pBindings = m_bindings.data();
    vkCreateDescriptorSetLayout(m_context.device, &layout_create_info, nullptr, &layout);

    m_layouts.push_back(layout);

    return layout;
}

}
