#include <iostream>
#include <cstdlib>
#include <ctime>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "gfx.hpp"
#include <vector>
#include <limits>
#include <optional>
#include <fstream>
#include <array>

#include "file.hpp"
#include "vk.hpp"

//#include <vulkan/vulkan_win32.h>

#define WIDTH	800
#define HEIGHT	800

static double mouse_x, mouse_y;
static double mouse_down_x, mouse_down_y;
static bool is_mouse_down;
static double center_x, center_y;
static double last_update_x, last_update_y;
static double zoom = 1.0;

namespace app
{

	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		mouse_x = xpos;
		mouse_y = ypos;

		if (is_mouse_down)
		{
			double dx = mouse_x - last_update_x;
			double dy = mouse_y - last_update_y;

			double ndx = dx / 200.0 / zoom;
			double ndy = dy / 200.0 / zoom;

			center_x -= ndx;
			center_y -= ndy;

			last_update_x = mouse_x;
			last_update_y = mouse_y;
		}
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		double constexpr factor = 1.1;

		if (yoffset > 0)
		{
			zoom *= factor;
		}
		else
		{
			zoom /= factor;
		}
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			is_mouse_down = true;
			last_update_x = mouse_x;
			last_update_y = mouse_y;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			is_mouse_down = false;
			// center_x += (mouse_down_x - mouse_x);
			// center_y += (mouse_down_y - center_y) / 200.0;
		}
	}

	app::window::window()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfw_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetCursorPosCallback(glfw_window, cursor_position_callback);
		glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
		glfwSetScrollCallback(glfw_window, scroll_callback);
		handle = glfwGetWin32Window(glfw_window);
	}

	app::window::~window()
	{
		glfwDestroyWindow(glfw_window);
		glfwTerminate();

	}


	
	static void vk_check(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan error!");
		}
	}

	app::gfx::gfx() :
		m_window(),
		m_instance(),
		m_physical_device(m_instance),
		m_surface(m_instance, m_window.handle),
		m_device(m_physical_device, m_surface),
		m_graphics_queue(m_device, m_device.queue_family_index),
		m_present_queue(m_device, m_device.queue_family_index)
	{
		set_up_swap_chain();
		set_up_command_pool();
		set_up_shaders();
		set_up_descriptor_pool();
		set_up_pipeline();

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(m_device, &fence_create_info, nullptr, &m_in_flight_fence);

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_image_available_semaphore);
		vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);	}

	app::gfx::~gfx()
	{
		vkDeviceWaitIdle(m_device);

		vkDestroySemaphore(m_device, m_render_finished_semaphore, nullptr);
		vkDestroySemaphore(m_device, m_image_available_semaphore, nullptr);
		vkDestroyFence(m_device, m_in_flight_fence, nullptr);

		tear_down_pipeline();
		tear_down_descriptor_pool();
		tear_down_shaders();
		tear_down_command_pool();
		tear_down_swap_chain();
		}

	void app::gfx::set_up_swap_chain()
	{
		VkExtent2D extent{};
		extent.width = WIDTH;
		extent.height = HEIGHT;
		
		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = m_surface;
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

		vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain);

		uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
		m_swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, m_swapchain_images.data());

		m_swapchain_image_views.resize(swapchain_image_count);

		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			VkImageViewCreateInfo image_view_create_info{};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = m_swapchain_images[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.subresourceRange.levelCount = 1;

			vkCreateImageView(m_device, &image_view_create_info, nullptr, &m_swapchain_image_views[i]);
		}
	}

	void app::gfx::tear_down_swap_chain()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_swapchain_image_views.size()); i++)
		{
			vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
		}
		
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	}

	void app::gfx::set_up_command_pool()
	{
		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = m_queue_family_index;

		vkCreateCommandPool(m_device, &create_info, nullptr, &m_command_pool);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandBufferCount = 1;
		alloc_info.commandPool = m_command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer);

	}

	void app::gfx::tear_down_command_pool()
	{
		vkDestroyCommandPool(m_device, m_command_pool, nullptr);
	}

	void app::gfx::set_up_descriptor_pool()
	{
		VkDescriptorPoolSize types;
		types.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		types.descriptorCount = 1;
		
		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = 0;
		create_info.pPoolSizes = &types;
		create_info.maxSets = 10; // TODO real value here.
		
		vkCreateDescriptorPool(m_device, &create_info, nullptr, &m_descriptor_pool);

		VkDescriptorSetLayoutBinding binding{};
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		binding.descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo layout_create_info{};
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.bindingCount = 1;
		layout_create_info.pBindings = &binding;
		vkCreateDescriptorSetLayout(m_device, &layout_create_info, nullptr, &m_layout);

		//
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = m_descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &m_layout;

		vk_check(vkAllocateDescriptorSets(m_device, &alloc_info, &m_descriptor_set));

	}

	void app::gfx::tear_down_descriptor_pool()
	{
		vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
		vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
	}

	void app::gfx::set_up_pipeline()
	{
		//VkDescriptorSetLayout layout{};
		////layout.

		//VkDescriptorSetLayoutCreateInfo layout_create_info{};
		//layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;		

		//vkCreateDescriptorSetLayout(m_device, &layout_create_info, nullptr, &layout);

		//VkDescriptorSetAllocateInfo alloc_info{};
		//alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//alloc_info.descriptorPool = m_descriptor_pool;
		//alloc_info.descriptorSetCount = 1;
		//alloc_info.pSetLayouts = &layout;

		//vkAllocateDescriptorSets()

		VkPipelineShaderStageCreateInfo vertex_stage_create_info{};
		vertex_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_stage_create_info.pName = "main";
		vertex_stage_create_info.module = m_vertex_shader_module;

		VkPipelineShaderStageCreateInfo fragment_stage_create_info{};
		fragment_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_stage_create_info.pName = "main";
		fragment_stage_create_info.module = m_fragment_shader_module;

		std::array<VkPipelineShaderStageCreateInfo, 2> stages = { vertex_stage_create_info, fragment_stage_create_info };

		VkPushConstantRange range{};
		range.size = 12;
		range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		//VkPipelineLayout pipeline_layout;
		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		//pipeline_layout_create_info.pPushConstantRanges = &range;
		//pipeline_layout_create_info.pushConstantRangeCount = 1;
		pipeline_layout_create_info.pSetLayouts = &m_layout;
		pipeline_layout_create_info.setLayoutCount = 1;
		vk_check(vkCreatePipelineLayout(m_device, &pipeline_layout_create_info, nullptr, &m_pipeline_layout));

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
		viewport.width = static_cast<float>(WIDTH);
		viewport.height = static_cast<float>(HEIGHT);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent.width = WIDTH;
		scissor.extent.height = HEIGHT;		

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
		create_info.layout = m_pipeline_layout;
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
		
		vkCreateGraphicsPipelines(m_device, nullptr, 1, &create_info, nullptr, &m_pipeline);

		//vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
	}

	void app::gfx::tear_down_pipeline()
	{
		vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
	}

	void app::gfx::set_up_shaders()
	{
		std::vector<char> buffer;

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		
		buffer = app::read_file("./shaders/vertex/simple.spv");		
		create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
		create_info.codeSize = buffer.size();
		vkCreateShaderModule(m_device, &create_info, nullptr, &m_vertex_shader_module);

		buffer = app::read_file("./shaders/fragment/simple.spv");
		create_info.pCode = reinterpret_cast<uint32_t*>(buffer.data());
		create_info.codeSize = buffer.size();
		vkCreateShaderModule(m_device, &create_info, nullptr, &m_fragment_shader_module);
	}

	void app::gfx::tear_down_shaders()
	{
		vkDestroyShaderModule(m_device, m_vertex_shader_module, nullptr);
		vkDestroyShaderModule(m_device, m_fragment_shader_module, nullptr);
	}

	void app::gfx::update()
	{
		std::srand(std::time(nullptr));

		while (!glfwWindowShouldClose(m_window.glfw_window))
		{
			uint32_t image_view_index = 0; // TODO GET THE INDEX
			constexpr uint32_t buffer_size = 128 * 4;

			glfwPollEvents();

			vkWaitForFences(m_device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
			vkResetCommandBuffer(m_command_buffer, 0);
			vkResetFences(m_device, 1, &m_in_flight_fence);

			VkBufferCreateInfo buffer_create_info{};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_create_info.size = buffer_size;
			buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

			VkBuffer buffer;
			vkCreateBuffer(m_device, &buffer_create_info, nullptr, &buffer);

			VkPhysicalDeviceMemoryProperties mem_properties;
			vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

			VkMemoryAllocateInfo memory_allocate_info{};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = buffer_size;
			memory_allocate_info.memoryTypeIndex = 2; // hard coded host-visible/coherent on my machine.

			VkDeviceMemory device_buffer_memory;
			vkAllocateMemory(m_device, &memory_allocate_info, nullptr, &device_buffer_memory);

			VkBindBufferMemoryInfo bind_buffer_memory_info{};
			bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
			bind_buffer_memory_info.memory = device_buffer_memory;
			bind_buffer_memory_info.buffer = buffer;
			vkBindBufferMemory(m_device, buffer, device_buffer_memory, 0);
			
			VkBufferViewCreateInfo buffer_view_create_info{};
			buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			buffer_view_create_info.format = VK_FORMAT_R32_UINT;
			buffer_view_create_info.range = buffer_size;
			buffer_view_create_info.buffer = buffer;

			VkBufferView buffer_view;
			vkCreateBufferView(m_device, &buffer_view_create_info, nullptr, &buffer_view);

			//vkBindBufferMemory

			float* mem;
			vk_check(vkMapMemory(m_device, device_buffer_memory, 0, buffer_size, 0, reinterpret_cast<void**>(&mem)));

			for (uint32_t i = 0; i < 100; i += 1)
			{
				mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);
			}

			// Fill out memory here.
			vkUnmapMemory(m_device, device_buffer_memory);

			
			VkDescriptorBufferInfo descriptor_buffer_info{};
			descriptor_buffer_info.buffer = buffer;
			descriptor_buffer_info.offset = 0;
			descriptor_buffer_info.range = buffer_size;

			VkWriteDescriptorSet write_descriptor_set{};
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = m_descriptor_set;
			write_descriptor_set.dstBinding = 0;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

			vkUpdateDescriptorSets(m_device, 1, &write_descriptor_set, 0, nullptr);

			vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_available_semaphore, nullptr, &image_view_index);
			
			VkClearValue clear_value{};

			VkRect2D render_area{};
			render_area.extent.width = WIDTH;
			render_area.extent.height = HEIGHT;

			VkRenderingAttachmentInfo color_attachment_info{};
			color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			color_attachment_info.clearValue = clear_value;
			color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
			color_attachment_info.imageView = m_swapchain_image_views[image_view_index];
			color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			VkRenderingInfo rendering_info{};
			rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			rendering_info.pColorAttachments = &color_attachment_info;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.layerCount = 1;
			rendering_info.renderArea = render_area;

			VkCommandBufferBeginInfo begin_info{};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(m_command_buffer, &begin_info);

			vkCmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_set, 0, nullptr);

			VkImageMemoryBarrier barrier1{};
			barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier1.image = m_swapchain_images[image_view_index];
			barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier1.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier1.subresourceRange.baseArrayLayer = 0;
			barrier1.subresourceRange.baseMipLevel = 0;
			barrier1.subresourceRange.layerCount = 1;
			barrier1.subresourceRange.levelCount = 1;
			vkCmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

			vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

			//float values[3] = { static_cast<float>(center_x), static_cast<float>(center_y), static_cast<float>(zoom)};
			//vkCmdPushConstants(m_command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 12, values);
			
			vkCmdBeginRendering(m_command_buffer, &rendering_info);
			vkCmdDraw(m_command_buffer, 6, 1, 0, 0);
			vkCmdEndRendering(m_command_buffer);
			
			//VkSemaphoreWaitInfo 

			//VkSemaphoreWait()

			VkImageMemoryBarrier barrier2{};
			barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier2.image = m_swapchain_images[image_view_index];
			barrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier2.subresourceRange.baseArrayLayer = 0;
			barrier2.subresourceRange.baseMipLevel = 0;
			barrier2.subresourceRange.layerCount = 1;
			barrier2.subresourceRange.levelCount = 1;
			vkCmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

			vkEndCommandBuffer(m_command_buffer);

			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			VkSubmitInfo submit_info{};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &m_command_buffer;
			submit_info.pWaitSemaphores = &m_image_available_semaphore;
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitDstStageMask = &wait_stage;
			submit_info.pSignalSemaphores = &m_render_finished_semaphore;
			submit_info.signalSemaphoreCount = 1;

			//submit_info.
			
			vkQueueSubmit(m_present_queue, 1, &submit_info, m_in_flight_fence);

			VkPresentInfoKHR present_info{};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pSwapchains = &m_swapchain;
			present_info.swapchainCount = 1;
			present_info.pWaitSemaphores = &m_render_finished_semaphore;
			present_info.waitSemaphoreCount = 1;
			present_info.pImageIndices = &image_view_index;

			vkQueuePresentKHR(m_present_queue, &present_info);

			// NO NO NO
			vkDeviceWaitIdle(m_device);

			vkDestroyBufferView(m_device, buffer_view, nullptr);
			vkDestroyBuffer(m_device, buffer, nullptr);
			vkFreeMemory(m_device, device_buffer_memory, nullptr);

		}
	}
}
