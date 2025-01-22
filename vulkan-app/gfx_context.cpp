#include <iostream>
#include <array>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <optional>

#include "file.hpp"
#include "vk.hpp"
#include "gfx_context.hpp"

static void check(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error!");
	}
}

/// <summary>
/// Initialize the context
/// </summary>
/// <param name="window_handle"></param>
/// <param name="width"></param>
/// <param name="height"></param>
gfx::context::context(HWND window_handle, uint32_t width, uint32_t height)
{
	create_instance();
	create_surface(window_handle);
	get_physical_device();
	create_device();
	vkGetDeviceQueue(device, graphics_queue.family_index, 0, &graphics_queue.handle);
	create_swapchain(width, height);
	create_descriptor_pool();
	create_descriptor_set_layout();
	create_descriptor_set();
	create_fragment_shader();
	create_vertex_shader();
	create_pipeline_layout();
	create_pipeline();

	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	check(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &get_next_framebuffer_semaphore));
}

/// <summary>
/// Set up the Vulkan instance
/// </summary>
void gfx::context::create_instance()
{
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulcan App";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Unknown Engine.";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_3;

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
		
	check(vkCreateInstance(&create_info, nullptr, &instance));
}

/// <summary>
/// Get the Vulkan physical device
/// </summary>
void gfx::context::get_physical_device()
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
	physical_device = physical_devices[0];

	VkPhysicalDeviceMemoryProperties memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	constexpr VkMemoryPropertyFlags device_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	constexpr VkMemoryPropertyFlags host_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t found = 0;
	
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if (memory_properties.memoryTypes[i].propertyFlags == device_flags)
		{
			memory_type_device_local = i;
			found++;
		}
		else if (memory_properties.memoryTypes[i].propertyFlags == host_flags)
		{
			memory_type_host_coherent = i;
			found++;
		}
	}

	if (found != 2)
	{
		throw std::runtime_error("Could not find memory types!");
	}
}

/// <summary>
/// Create the surface
/// </summary>
/// <param name="window_handle"></param>
void gfx::context::create_surface(HWND window_handle)
{
	VkWin32SurfaceCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_handle;
	create_info.hinstance = GetModuleHandle(nullptr);

	check(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));	
}

/// <summary>
/// Create the logical device.
/// </summary>
void gfx::context::create_device()
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

	graphics_queue.family_index = queue_family_index_o.value();

	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.queueFamilyIndex = graphics_queue.family_index;
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

	vkCreateDevice(physical_device, &create_info, nullptr, &device);
}

/// <summary>
/// Create the swapchain
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
void gfx::context::create_swapchain(uint32_t width, uint32_t height)
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

	vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);

	uint32_t swapchain_image_count;
	std::vector<VkImage> images;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
	images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, images.data());

	framebuffers.resize(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		framebuffers[i].index = i;
		
		framebuffers[i].image = images[i];

		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = framebuffers[i].image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;

		vkCreateImageView(device, &image_view_create_info, nullptr, &framebuffers[i].view);
	}
}

void gfx::context::create_descriptor_pool()
{
	VkDescriptorPoolSize types;
	types.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	types.descriptorCount = 1;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.poolSizeCount = 0;
	create_info.pPoolSizes = &types;
	create_info.maxSets = 10; // TODO real value here.

	vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool);
}

void gfx::context::create_descriptor_set()
{
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &descriptor_set_layout;

	check(vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set));
}

void gfx::context::create_pipeline_layout()
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
	check(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout));
}

void gfx::context::create_fragment_shader()
{
	std::vector<char> buffer;

	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	buffer = app::read_file("./shaders/fragment/simple.spv");
	create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
	create_info.codeSize = buffer.size();
	vkCreateShaderModule(device, &create_info, nullptr, &fragment_shader);

}

void gfx::context::create_vertex_shader()
{
	std::vector<char> buffer;

	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	buffer = app::read_file("./shaders/vertex/simple.spv");
	create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
	create_info.codeSize = buffer.size();
	vkCreateShaderModule(device, &create_info, nullptr, &vertex_shader);

}

void gfx::context::create_pipeline()
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

	vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr, &pipeline);
}
 

/// <summary>
/// Destroy the context
/// </summary>
gfx::context::~context()
{
	vkDeviceWaitIdle(device);

	for (auto device_memory : allocated_device_memory)
	{
		vkFreeMemory(device, device_memory, nullptr);
	}

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	vkDestroyShaderModule(device, fragment_shader, nullptr);
	vkDestroyShaderModule(device, vertex_shader, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
	vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
	vkDestroySemaphore(device, get_next_framebuffer_semaphore, nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(framebuffers.size()); i++)
	{
		vkDestroyImageView(device, framebuffers[i].view, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

/// <summary>
/// Gets the next framebuffer in the swapchain.
/// Sets the get_next_framebuffer_semaphore semaphore.
/// </summary>
/// <returns></returns>
gfx::framebuffer& gfx::context::get_next_framebuffer()
{
	uint32_t image_index;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, get_next_framebuffer_semaphore, nullptr, &image_index);
	return framebuffers[image_index];
}

void gfx::context::create_descriptor_set_layout()
{
	VkDescriptorSetLayoutBinding binding{};
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	binding.descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = &binding;
	check(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &descriptor_set_layout));
}

void gfx::context::begin_command_buffer(VkCommandBuffer command_buffer)
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(command_buffer, &begin_info);
}

void gfx::context::begin_rendering(VkCommandBuffer command_buffer, VkImageView image_view)
{
	VkClearValue clear_value{};

	VkRect2D render_area{};
	render_area.extent.width = 800;
	render_area.extent.height = 800;

	VkRenderingAttachmentInfo color_attachment_info{};
	color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color_attachment_info.clearValue = clear_value;
	color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
	color_attachment_info.imageView = image_view;
	color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.pColorAttachments = &color_attachment_info;
	rendering_info.colorAttachmentCount = 1;
	rendering_info.layerCount = 1;
	rendering_info.renderArea = render_area;

	vkCmdBeginRendering(command_buffer, &rendering_info);
}

void gfx::context::transition_image(VkCommandBuffer command_buffer,
	                                VkImage image,
	                                VkShaderStageFlags source_stage,
	                                VkAccessFlags source_access_mask,
	                                VkShaderStageFlags desintation_stage,
	                                VkAccessFlags destination_access_mask,
	                                VkImageLayout old_layout,
	                                VkImageLayout new_layout)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	vkCmdPipelineBarrier(command_buffer, source_stage, desintation_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

}

void gfx::context::present(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, uint32_t image_index)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &swapchain;
	present_info.swapchainCount = 1;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = 1;
	present_info.pImageIndices = &image_index;
	vkQueuePresentKHR(graphics_queue.handle, &present_info);
}


void gfx::context::submit(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_stage, VkSemaphore signal_semaphore, VkFence fence)
{
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.pWaitSemaphores = &wait_semaphore;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.pSignalSemaphores = &signal_semaphore;
	submit_info.signalSemaphoreCount = 1;
	vkQueueSubmit(graphics_queue.handle, 1, &submit_info, fence);
}


void gfx::context::upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size)
{
	VkDeviceMemory device_memory;

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = buffer_size;
	memory_allocate_info.memoryTypeIndex = memory_type_host_coherent;
	check(vkAllocateMemory(device, &memory_allocate_info, nullptr, &device_memory));

	VkBindBufferMemoryInfo bind_buffer_memory_info{};
	bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
	bind_buffer_memory_info.memory = device_memory;
	bind_buffer_memory_info.buffer = buffer;
	check(vkBindBufferMemory(device, buffer, device_memory, 0));

	void* mem;
	check(vkMapMemory(device, device_memory, 0, buffer_size, 0, &mem));
	memcpy(mem, source, buffer_size);
	vkUnmapMemory(device, device_memory);

	allocated_device_memory.push_back(device_memory);

	// TODO: free memory at end of app.
	// There's no validation error on this so maybe I don't need to.
}