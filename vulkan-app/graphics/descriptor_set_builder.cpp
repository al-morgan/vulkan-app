#include <vulkan/vulkan.h>

#include "graphics/context.hpp"
#include "graphics/descriptor_set_cache.hpp"

namespace graphics
{

descriptor_set_builder::descriptor_set_builder(graphics::context& context) :
    m_context(context)
{
    VkDescriptorPoolSize types;
    types.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    types.descriptorCount = 1;

    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 0;
    create_info.pPoolSizes = &types;
    create_info.maxSets = 10;

    vkCreateDescriptorPool(context.device, &create_info, nullptr, &m_descriptor_pool);
}

void descriptor_set_builder::reset()
{
    m_bindings.clear();
    m_layout = VK_NULL_HANDLE;
}

void descriptor_set_builder::add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding dsl_binding{};
    dsl_binding.binding = binding;
    dsl_binding.descriptorType = descriptor_type;
    dsl_binding.stageFlags = stage_flags;
    dsl_binding.descriptorCount = 1;
    m_bindings.push_back(dsl_binding);
}

VkDescriptorSet descriptor_set_builder::get_result()
{
    // TODO: check if descriptor set exists.
    VkDescriptorSet descriptor_set;

    VkDescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.bindingCount = m_bindings.size();
    layout_create_info.pBindings = m_bindings.data();
    vkCreateDescriptorSetLayout(m_context.device, &layout_create_info, nullptr, &m_layout);

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_layout;
    vkAllocateDescriptorSets(m_context.device, &alloc_info, &descriptor_set);

    return descriptor_set;
}

}
