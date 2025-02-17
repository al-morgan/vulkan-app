#pragma once

#include <vector>

#include "vulkan/vulkan.h"

#include "graphics/buffer.hpp"
#include "graphics/image.hpp"

namespace graphics
{

class pass
{
private:
    // What I need:
    // Descriptor sets
    // Transitions

public:
    graphics::context& m_context;
    VkPipeline m_pipeline;
    std::vector<std::reference_wrapper<graphics::buffer>> m_buffers;
    std::vector<std::reference_wrapper<graphics::image>> m_images;
    //std::vector<graphics::descriptor_set>& m_descriptor_sets;
    VkPipelineLayout m_pipeline_layout;
    std::vector<VkDescriptorType> m_descriptor_types;

    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;

    void add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
    {
        VkDescriptorSetLayoutBinding dsl_binding{};
        dsl_binding.binding = binding;
        dsl_binding.descriptorType = descriptor_type;
        dsl_binding.stageFlags = stage_flags;
        dsl_binding.descriptorCount = 1;
        m_bindings.push_back(dsl_binding);

        m_descriptor_types.resize(binding + 1);
        m_descriptor_types[binding] = descriptor_type;
    }

    void commit()
    {
        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount = m_bindings.size();
        layout_create_info.pBindings = m_bindings.data();
        layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
        vkCreateDescriptorSetLayout(m_context.device, &layout_create_info, nullptr, &m_layout);

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_context.descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &m_layout;
        vkAllocateDescriptorSets(m_context.device, &alloc_info, &m_descriptor_set);
    }

    VkDescriptorSetLayout layout() { return m_layout; }


    void update(uint32_t binding, graphics::buffer& buffer)
    {
        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = buffer.handle();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = buffer.size();

        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = m_descriptor_set;
        write_descriptor_set.dstBinding = binding;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.descriptorType = m_descriptor_types[binding];
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        vkUpdateDescriptorSets(m_context.device, 1, &write_descriptor_set, 0, nullptr);
    }

    void finalize(VkShaderModule vertex_shader, VkShaderModule fragment_shader)
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pSetLayouts = &m_layout;
        pipeline_layout_create_info.setLayoutCount = 1;
        vkCreatePipelineLayout(m_context.device, &pipeline_layout_create_info, nullptr, &m_pipeline_layout);

        VkPipelineShaderStageCreateInfo vertex_stage_create_info{};
        vertex_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_stage_create_info.pName = "main";
        vertex_stage_create_info.module = vertex_shader;

        VkPipelineShaderStageCreateInfo fragment_stage_create_info{};
        fragment_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_stage_create_info.pName = "main";
        fragment_stage_create_info.module = fragment_shader;

        std::array<VkPipelineShaderStageCreateInfo, 2> stages = { vertex_stage_create_info, fragment_stage_create_info };

        VkVertexInputAttributeDescription vertex_input_attribute_description{};
        vertex_input_attribute_description.binding = 0;
        vertex_input_attribute_description.location = 0;
        vertex_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
        vertex_input_attribute_description.offset = 0;

        VkVertexInputBindingDescription vertex_input_binding_description{};
        vertex_input_binding_description.binding = 0;
        vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertex_input_binding_description.stride = sizeof(glm::vec3);

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
        input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineMultisampleStateCreateInfo multisample_state{};
        multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineRasterizationStateCreateInfo rasterization_state{};
        rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state.lineWidth = 1.0f;
        rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
        //rasterization_state.polygonMode = VK_POLYGON_MODE_LINE;

        VkViewport viewport{};
        viewport.width = static_cast<float>(800);
        viewport.height = static_cast<float>(800);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.extent.width = 800;
        scissor.extent.height = 800;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.pViewports = &viewport;
        viewport_state.viewportCount = 1;
        viewport_state.pScissors = &scissor;
        viewport_state.scissorCount = 1;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = &color_blend_attachment_state;
        color_blend_state.logicOp = VK_LOGIC_OP_SET;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
        depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state.depthTestEnable = VK_TRUE;
        depth_stencil_state.depthWriteEnable = VK_TRUE;
        depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;

        VkGraphicsPipelineCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.stageCount = 2;
        create_info.pStages = stages.data();
        create_info.layout = m_pipeline_layout;
        create_info.pVertexInputState = &vertex_input_state_create_info;
        create_info.pInputAssemblyState = &input_assembly_state;
        create_info.pMultisampleState = &multisample_state;
        create_info.pRasterizationState = &rasterization_state;
        create_info.pViewportState = &viewport_state;
        create_info.pColorBlendState = &color_blend_state;
        create_info.pDepthStencilState = &depth_stencil_state;

        // Tutorial says I need this but it works without it?
        VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
        VkPipelineRenderingCreateInfo pipeline_rendering_create_info{};
        pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_create_info.colorAttachmentCount = 1;
        pipeline_rendering_create_info.pColorAttachmentFormats = &format;
        pipeline_rendering_create_info.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        create_info.pNext = &pipeline_rendering_create_info;

        vkCreateGraphicsPipelines(m_context.device, nullptr, 1, &create_info, nullptr, &m_pipeline);


    }
    pass(graphics::context& context) : m_context(context)
    {

    }
};

}
