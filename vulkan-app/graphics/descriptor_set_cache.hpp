#pragma once

#include <vector>

#include "vulkan/vulkan.h"

#include "graphics/context.hpp"

namespace graphics
{

class descriptor_set_cache
{
public:
    descriptor_set_cache(graphics::context& context) :
        m_context(context)
    {

    }

    void reset()
    {
        m_bindings.clear();
        m_layout = VK_NULL_HANDLE;
    }

    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
    {
        VkDescriptorSetLayoutBinding dsl_binding{};
        dsl_binding.binding = binding;
        dsl_binding.descriptorType = descriptor_type;
        dsl_binding.stageFlags = stage_flags;
        dsl_binding.descriptorCount = 1;
        m_bindings.push_back(dsl_binding);
    }

    void commit()
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
        alloc_info.descriptorPool = m_context.descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &m_layout;
        vkAllocateDescriptorSets(m_context.device, &alloc_info, &descriptor_set);

        m_descriptor_sets.push_back(descriptor_set);
    }

private:
    std::vector<VkDescriptorSet> m_descriptor_sets;
    graphics::context& m_context;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;

    // This should be an unordered map.
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};


};