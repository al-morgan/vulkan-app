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

vk::instance::instance()
{
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulcan App";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Unknown Engine.";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

	//uint32_t glfw_extension_count;
	//const char** glfw_extensions;
	//glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	std::vector<const char*> enabled_extensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };

	uint32_t instance_extension_count = 0;
	std::vector<VkExtensionProperties> instance_extensions;
	check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
	instance_extensions.resize(instance_extension_count);
	check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

	uint32_t instance_layer_count;
	std::vector<VkLayerProperties> instance_layers;
	check(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
	instance_layers.resize(instance_layer_count);
	check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

	std::vector<const char*> enabled_layers = { "VK_LAYER_KHRONOS_validation" };

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.ppEnabledExtensionNames = enabled_extensions.data();
	create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
	create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
	create_info.ppEnabledLayerNames = enabled_layers.data();
	check(vkCreateInstance(&create_info, nullptr, &handle));
}

vk::instance::~instance()
{
	vkDestroyInstance(handle, nullptr);
}

vk::physical_device::physical_device(vk::instance& instance)
{
	uint32_t physical_device_count;
	std::vector<VkPhysicalDevice> physical_devices;
	check(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
	physical_devices.resize(physical_device_count);
	check(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));

	if (physical_device_count != 1)
	{
		throw std::runtime_error("Multiple physical devices not supported!");
	}

	// I only have one physical device right now so I'm going to cheat
	handle = physical_devices[0];
}


vk::surface::surface(vk::instance& instance, HWND window_handle)
{
	instance_handle = instance;
	
	VkWin32SurfaceCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_handle;
	create_info.hinstance = GetModuleHandle(nullptr);

	check(vkCreateWin32SurfaceKHR(instance_handle, &create_info, nullptr, &handle));
}

vk::surface::~surface()
{
	vkDestroySurfaceKHR(instance_handle, handle, nullptr);
}

vk::device::device(vk::physical_device& physical_device, vk::surface& surface)
{
	uint32_t queue_family_count;
	std::vector<VkQueueFamilyProperties> queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
	std::optional<uint32_t> queue_family_index_o;

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		constexpr VkFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
		VkBool32 surface_support = VK_FALSE;

		check(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &surface_support));
		if ((queue_families[i].queueFlags & required_flags) == required_flags && surface_support)
		{
			queue_family_index_o = i;
			break;
		}
	}

	if (!queue_family_index_o.has_value())
	{
		throw std::runtime_error("Queue selection failed!");
	}

	queue_family_index = queue_family_index_o.value();

	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.queueFamilyIndex = queue_family_index;
	device_queue_create_info.pQueuePriorities = &queue_priority;

	std::vector<const char*> enabled_extensions = { "VK_KHR_swapchain", "VK_KHR_dynamic_rendering" };

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = &dynamic_rendering_features;
	create_info.pQueueCreateInfos = &device_queue_create_info;
	create_info.queueCreateInfoCount = 1;
	create_info.ppEnabledExtensionNames = enabled_extensions.data();
	create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());

	vkCreateDevice(physical_device, &create_info, nullptr, &handle);
}

vk::device::~device()
{
	vkDestroyDevice(handle, nullptr);
}

vk::queue::queue(vk::device& device, uint32_t queue_family_index)
{
	vkGetDeviceQueue(device, queue_family_index, 0, &handle);
}

vk::swapchain::swapchain(vk::device& device, vk::surface& surface, uint32_t width, uint32_t height) : m_device(device)
{
	VkExtent2D extent{};
	extent.width = width;
	extent.height = height;


	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;
	create_info.minImageCount = 3;
	create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(device, &create_info, nullptr, &handle);

	uint32_t swapchain_image_count;
	vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, nullptr);
	images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, images.data());

	image_views.resize(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = images[i];
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;

		vkCreateImageView(device, &image_view_create_info, nullptr, &image_views[i]);
	}
}

vk::swapchain::~swapchain()
{
	for (uint32_t i = 0; i < static_cast<uint32_t>(image_views.size()); i++)
	{
		vkDestroyImageView(m_device, image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, handle, nullptr);
}

vk::command_pool::command_pool(vk::device& device, uint32_t queue_family_index) : device(device)
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

vk::command_buffer::command_buffer(vk::device& device, vk::command_pool& command_pool) : device(device), command_pool(command_pool)
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

vk::shader_module::shader_module(vk::device& device, std::string filename) : device(device)
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

vk::descriptor_pool::descriptor_pool(vk::device& device) : device(device)
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

vk::descriptor_set_layout::descriptor_set_layout(vk::device& device) : device(device)
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

vk::descriptor_set::descriptor_set(vk::device& device, vk::descriptor_pool& pool, vk::descriptor_set_layout& layout) {
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

vk::pipeline_layout::pipeline_layout(vk::device& device, vk::descriptor_set_layout& descriptor_set_layout) : device(device)
{


	VkPushConstantRange range{};
	range.size = 12;
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//VkPipelineLayout pipeline_layout;
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//pipeline_layout_create_info.pPushConstantRanges = &range;
	//pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pSetLayouts = descriptor_set_layout;
	pipeline_layout_create_info.setLayoutCount = 1;
	check(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &handle));
}

vk::pipeline_layout::~pipeline_layout()
{
	vkDestroyPipelineLayout(device, handle, nullptr);
}


vk::pipeline::pipeline(vk::device& device, vk::pipeline_layout &pipeline_layout, vk::shader_module& vertex_shader, vk::shader_module& fragment_shader, uint32_t width, uint32_t height) : device(device)
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

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
	vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
