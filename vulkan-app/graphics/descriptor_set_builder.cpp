#include <vulkan/vulkan.h>

#include "graphics/descriptor_set_builder.hpp"
#include "graphics/canvas.hpp"

namespace graphics
{

descriptor_set_builder::descriptor_set_builder(graphics::canvas& context) :
    m_context(context)
{
    VkDescriptorPoolSize types;
    types.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    types.descriptorCount = 1;

    // This had zero pool sizes? Did this even work?
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 0;
    create_info.pPoolSizes = &types;
    create_info.maxSets = 10;

    vkCreateDescriptorPool(m_context.device, &create_info, nullptr, &m_descriptor_pool);
}

descriptor_set_builder::~descriptor_set_builder()
{
    vkDestroyDescriptorPool(m_context.device, m_descriptor_pool, nullptr);
}

void descriptor_set_builder::set_layout(VkDescriptorSetLayout layout)
{
    m_layout = layout;
}


VkDescriptorSet descriptor_set_builder::get_result()
{
    VkDescriptorSet descriptor_set;
    
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_layout;
    vkAllocateDescriptorSets(m_context.device, &alloc_info, &descriptor_set);

    return descriptor_set;
}

}
