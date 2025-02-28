#include <vulkan/vulkan.h>

#include "graphics/set_builder.hpp"
#include "graphics/canvas.hpp"

namespace graphics
{
set_builder::set_builder(graphics::canvas& context) :
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

    vkCreateDescriptorPool(context.device, &create_info, nullptr, &m_pool);

}

set_builder::~set_builder()
{
    vkDestroyDescriptorPool(m_context.device, m_pool, nullptr);
}

void set_builder::reset()
{
    m_layouts.clear();
}

void set_builder::add_set(uint32_t set, VkDescriptorSetLayout layout)
{
    m_layouts.push_back(layout);
}

VkDescriptorSet set_builder::get_result()
{
    VkDescriptorSet descriptor_set;

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_pool;
    alloc_info.descriptorSetCount = m_layouts.size();
    alloc_info.pSetLayouts = m_layouts.data();

    vkAllocateDescriptorSets(m_context.device, &alloc_info, &descriptor_set);

    //m_descriptor_sets.push_back(descriptor_set);

    return descriptor_set;

}




}