#include <vulkan/vulkan.h>

#include "graphics/pipeline_layout_builder.hpp"

namespace graphics
{

pipeline_layout_builder::pipeline_layout_builder(graphics::canvas& context) :
    m_context(context)
{ }

pipeline_layout_builder::~pipeline_layout_builder()
{
    for (auto layout : m_layouts)
    {
        vkDestroyPipelineLayout(m_context.device, layout, nullptr);
    }
}

void pipeline_layout_builder::reset()
{
    m_set_layouts.clear();
}

void pipeline_layout_builder::add_set(uint32_t layout, VkDescriptorSetLayout descriptor_set)
{
    m_set_layouts.push_back(descriptor_set);
}

VkPipelineLayout pipeline_layout_builder::get_result()
{
    VkPipelineLayout layout;
    
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pSetLayouts = m_set_layouts.data();
    create_info.setLayoutCount = m_set_layouts.size();
    
    vkCreatePipelineLayout(m_context.device, &create_info, nullptr, &layout);

    m_layouts.push_back(layout);

    return layout;
}


}
