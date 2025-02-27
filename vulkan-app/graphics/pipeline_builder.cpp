#include <array>

#include <vulkan/vulkan.h>

#include "graphics/pipeline_builder.hpp"
#include "graphics/graphics.hpp"

namespace graphics
{

pipeline_builder::pipeline_builder(graphics::context& context) :
    m_context(context)
{

}

pipeline_builder::~pipeline_builder()
{
    for (auto pipeline : m_pipelines)
    {
        vkDestroyPipeline(m_context.device, pipeline, nullptr);
    }
}

void pipeline_builder::reset()
{

}

void pipeline_builder::set_layout(VkPipelineLayout layout)
{
    m_layout = layout;
}

void pipeline_builder::set_vertex_shader(VkShaderModule shader)
{
    m_vertex_shader = shader;
}

void pipeline_builder::set_fragment_shader(VkShaderModule shader)
{
    m_fragment_shader = shader;
}

VkPipeline pipeline_builder::get_result()
{
    VkPipeline pipeline;

    VkPipelineShaderStageCreateInfo vertex_stage_create_info{};
    vertex_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage_create_info.pName = "main";
    vertex_stage_create_info.module = m_vertex_shader;

    VkPipelineShaderStageCreateInfo fragment_stage_create_info{};
    fragment_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage_create_info.pName = "main";
    fragment_stage_create_info.module = m_fragment_shader;

    std::array<VkPipelineShaderStageCreateInfo, 2> stages = { vertex_stage_create_info, fragment_stage_create_info };

    // TODO: Make this configurable.
    VkVertexInputAttributeDescription vertex_input_attribute_descriptions[2];
    vertex_input_attribute_descriptions[0].binding = 0;
    vertex_input_attribute_descriptions[0].location = 0;
    vertex_input_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attribute_descriptions[0].offset = 0;

    vertex_input_attribute_descriptions[1].binding = 0;
    vertex_input_attribute_descriptions[1].location = 1;
    vertex_input_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attribute_descriptions[1].offset = 12;

    // TODO: Make this configurable
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_input_binding_description.stride = sizeof(graphics::vertex3d);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 2;
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
    create_info.layout = m_layout;
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

    vkCreateGraphicsPipelines(m_context.device, nullptr, 1, &create_info, nullptr, &pipeline);

    m_pipelines.push_back(pipeline);

    return pipeline;
}

}
