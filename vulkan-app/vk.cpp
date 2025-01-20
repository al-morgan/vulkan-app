#include <iostream>
#include <cstdlib>
#include <ctime>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <limits>
#include <optional>
#include <fstream>
#include <array>

#include "file.hpp"
#include "vk.hpp"

//#include <vulkan/vulkan_win32.h>

//namespace vk
//{

static void check(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error!");
	}
}

vk::command_pool::command_pool(VkDevice device, uint32_t queue_family_index) : device(device)
{
	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = queue_family_index;

	check(vkCreateCommandPool(device, &create_info, nullptr, &handle));
}

vk::command_pool::~command_pool()
{
	vkDestroyCommandPool(device, handle, nullptr);
}

vk::command_buffer::command_buffer(VkDevice device, vk::command_pool& command_pool) : device(device), command_pool(command_pool)
{
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(device, &alloc_info, &handle);
}

vk::command_buffer::~command_buffer()
{
	vkFreeCommandBuffers(device, command_pool, 1, &handle);
}

vk::shader_module::shader_module(VkDevice device, std::string filename) : device(device)
{
	std::vector<char> buffer;

	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	buffer = app::read_file(filename);
	create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
	create_info.codeSize = buffer.size();
	vkCreateShaderModule(device, &create_info, nullptr, &handle);
}

vk::shader_module::~shader_module()
{
	vkDestroyShaderModule(device, handle, nullptr);
}

vk::descriptor_pool::descriptor_pool(VkDevice device) : device(device)
{
	VkDescriptorPoolSize types;
	types.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	types.descriptorCount = 1;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.poolSizeCount = 0;
	create_info.pPoolSizes = &types;
	create_info.maxSets = 10; // TODO real value here.

	vkCreateDescriptorPool(device, &create_info, nullptr, &handle);
}

vk::descriptor_pool::~descriptor_pool()
{
	vkDestroyDescriptorPool(device, handle, nullptr);
}

vk::descriptor_set_layout::descriptor_set_layout(VkDevice device) : device(device)
{
	VkDescriptorSetLayoutBinding binding{};
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	binding.descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = &binding;
	check(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &handle));
}

vk::descriptor_set_layout::~descriptor_set_layout()
{
	vkDestroyDescriptorSetLayout(device, handle, nullptr);
}

vk::descriptor_set::descriptor_set(VkDevice device, VkDescriptorPool pool, vk::descriptor_set_layout& layout) {
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = layout;

	check(vkAllocateDescriptorSets(device, &alloc_info, &handle));
}

vk::descriptor_set::~descriptor_set() {
	// Gets deallocated when pool is destroyed
}

vk::pipeline_layout::pipeline_layout(VkDevice device, VkDescriptorSetLayout descriptor_set_layout) : device(device)
{
	VkPushConstantRange range{};
	range.size = 12;
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//VkPipelineLayout pipeline_layout;
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//pipeline_layout_create_info.pPushConstantRanges = &range;
	//pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
	pipeline_layout_create_info.setLayoutCount = 1;
	check(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &handle));
}

vk::pipeline_layout::~pipeline_layout()
{
	vkDestroyPipelineLayout(device, handle, nullptr);
}

vk::pipeline::pipeline(VkDevice device, vk::pipeline_layout &pipeline_layout, vk::shader_module& vertex_shader, vk::shader_module& fragment_shader, uint32_t width, uint32_t height) : device(device)
{
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
	vertex_input_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
	vertex_input_attribute_description.offset = 0;

	VkVertexInputBindingDescription vertex_input_binding_description{};
	vertex_input_binding_description.binding = 0;
	vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertex_input_binding_description.stride = sizeof(vk::Vertex);
	
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

	VkViewport viewport{};
	viewport.width = static_cast<float>(width);
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent.width = width;
	scissor.extent.height = height;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pViewports = &viewport;
	viewport_state.viewportCount = 1;
	viewport_state.pScissors = &scissor;
	viewport_state.scissorCount = 1;

	VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
	color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	//color_blend_attachment_state.blendEnable = 
	//color_blend_attachment_state.alphaBlendOp = 

	VkPipelineColorBlendStateCreateInfo color_blend_state{};
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.attachmentCount = 1;
	color_blend_state.pAttachments = &color_blend_attachment_state;
	color_blend_state.logicOp = VK_LOGIC_OP_SET;
	//color_blend_state.blendConstants

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.stageCount = 2;
	create_info.pStages = stages.data();
	create_info.layout = pipeline_layout;
	create_info.pVertexInputState = &vertex_input_state_create_info;
	create_info.pInputAssemblyState = &input_assembly_state;
	create_info.pMultisampleState = &multisample_state;
	create_info.pRasterizationState = &rasterization_state;
	create_info.pViewportState = &viewport_state;
	create_info.pColorBlendState = &color_blend_state;
	//create_info.layout

	// Tutorial says I need this but it works without it?
	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
	VkPipelineRenderingCreateInfo pipeline_rendering_create_info{};
	pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipeline_rendering_create_info.colorAttachmentCount = 1;
	pipeline_rendering_create_info.pColorAttachmentFormats = &format;
	create_info.pNext = &pipeline_rendering_create_info;

	vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr, &handle);
}

vk::pipeline::~pipeline()
{
	vkDestroyPipeline(device, handle, nullptr);
}
