#include <string>

#include <vulkan/vulkan.h>

#include "file.hpp"
#include "graphics/canvas.hpp"
#include "graphics/program_builder.hpp"
#include "graphics/program.hpp"

namespace graphics
{

graphics::program_builder::program_builder(graphics::canvas& canvas) :
    m_canvas(canvas)
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

    vkCreateDescriptorPool(m_canvas, &create_info, nullptr, &m_descriptor_pool);

}

graphics::program_builder::~program_builder()
{
    for (auto shader_module : m_shader_modules)
    {
        vkDestroyShaderModule(m_canvas, shader_module, nullptr);
    }

    for (auto layout : m_set_layouts)
    {
        vkDestroyDescriptorSetLayout(m_canvas, layout, nullptr);
    }

    for (auto layout : m_pipeline_layouts)
    {
        vkDestroyPipelineLayout(m_canvas, layout, nullptr);
    }

    for (auto pipeline : m_pipelines)
    {
        vkDestroyPipeline(m_canvas, pipeline, nullptr);
    }

    vkDestroyDescriptorPool(m_canvas, m_descriptor_pool, nullptr);
}

void graphics::program_builder::add_stage(VkShaderStageFlagBits stage,
    std::string code)
{
    std::vector<char> buffer;
    VkShaderModule shader_module;

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    buffer = app::read_file(code);
    create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
    create_info.codeSize = buffer.size();
    vkCreateShaderModule(m_canvas, &create_info, nullptr, &shader_module);

    m_shader_modules.push_back(shader_module);

    VkPipelineShaderStageCreateInfo vertex_stage_create_info{};
    vertex_stage_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_create_info.stage = stage;
    vertex_stage_create_info.pName = "main";
    vertex_stage_create_info.module = shader_module;

    m_pipeline_shader_stage_cis.push_back(vertex_stage_create_info);
}

void program_builder::add_binding(uint32_t binding,
    VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
    m_bindings[m_current_set].resize(binding + 1);

    VkDescriptorSetLayoutBinding dsl_binding{};
    dsl_binding.binding = binding;
    dsl_binding.descriptorType = descriptor_type;
    dsl_binding.stageFlags = stage_flags;
    dsl_binding.descriptorCount = 1;
    m_bindings[m_current_set][binding] = dsl_binding;
}

void graphics::program_builder::add_set(uint32_t set)
{
    m_current_set = set;
    m_bindings.resize(set + 1);
}

graphics::program program_builder::get_program()
{
    VkPipelineLayout layout;

    for (auto set_layout : m_bindings)
    {
        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo dsl_ci{};
        dsl_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsl_ci.bindingCount = set_layout.size();
        dsl_ci.pBindings = set_layout.data();
        vkCreateDescriptorSetLayout(m_canvas, &dsl_ci, nullptr, &layout);
        m_set_layouts.push_back(layout);
    }

    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pSetLayouts = m_set_layouts.data();
    create_info.setLayoutCount = m_set_layouts.size();

    vkCreatePipelineLayout(m_canvas, &create_info, nullptr, &layout);

    m_pipeline_layouts.push_back(layout);

    VkPipeline pipeline;

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
    viewport.width = static_cast<float>(m_canvas.get_width());
    viewport.height = static_cast<float>(m_canvas.get_height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent.width = m_canvas.get_width();
    scissor.extent.height = m_canvas.get_height();

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

    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = m_pipeline_shader_stage_cis.size();
    pipeline_create_info.pStages = m_pipeline_shader_stage_cis.data();
    pipeline_create_info.layout = m_pipeline_layouts.back();
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;

    // Tutorial says I need this but it works without it?
    VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
    VkPipelineRenderingCreateInfo pipeline_rendering_create_info{};
    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipeline_rendering_create_info.colorAttachmentCount = 1;
    pipeline_rendering_create_info.pColorAttachmentFormats = &format;
    pipeline_rendering_create_info.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    pipeline_create_info.pNext = &pipeline_rendering_create_info;

    vkCreateGraphicsPipelines(m_canvas, nullptr, 1, &pipeline_create_info, nullptr, &pipeline);

    m_pipelines.push_back(pipeline);

    graphics::program program(pipeline, layout);

    return program;
}


VkDescriptorSet graphics::program_builder::get_descriptor_set(uint32_t set)
{
    VkDescriptorSet descriptor_set;

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_set_layouts[set];
    vkAllocateDescriptorSets(m_canvas, &alloc_info, &descriptor_set);

    return descriptor_set;
}


}
