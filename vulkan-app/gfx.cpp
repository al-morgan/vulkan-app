#include <iostream>
#include <cstdlib>
#include <ctime>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>


#include "gfx.hpp"
#include <vector>
#include <limits>
#include <optional>
#include <fstream>
#include <array>

#include "file.hpp"
#include "vk.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


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

	app::gfx::gfx(vk::device& device) : device(device)
		
		/*:
		m_window(),
		m_instance(),
		m_physical_device(m_instance),
		m_surface(m_instance, m_window.handle),
		m_device(m_physical_device, m_surface),
		m_graphics_queue(m_device, m_device.queue_family_index),
		m_present_queue(m_device, m_device.queue_family_index),
		m_swapchain(m_device, m_surface, WIDTH, HEIGHT),
		m_command_pool(m_device, m_device.queue_family_index),
		m_command_buffer(m_device, m_command_pool),
		m_fragment_shader_module(m_device, "./shaders/fragment/simple.spv"),
		m_vertex_shader_module(m_device, "./shaders/vertex/simple.spv"),
		m_descriptor_pool(m_device),
		m_layout(m_device),
		m_descriptor_set(m_device, m_descriptor_pool, m_layout),
		m_pipeline_layout(m_device, m_layout),
		m_pipeline(m_device, m_pipeline_layout, m_vertex_shader_module, m_fragment_shader_module, WIDTH, HEIGHT)*/

	{
		//set_up_pipeline();

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fence_create_info, nullptr, &m_in_flight_fence);

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(device, &semaphore_create_info, nullptr, &m_image_available_semaphore);
		vkCreateSemaphore(device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);	}

	app::gfx::~gfx()
	{
		vkDeviceWaitIdle(device);

		vkDestroySemaphore(device, m_render_finished_semaphore, nullptr);
		vkDestroySemaphore(device, m_image_available_semaphore, nullptr);
		vkDestroyFence(device, m_in_flight_fence, nullptr);
		}



	void app::gfx::update(app::window& window, vk::device& device, vk::command_buffer& command_buffer, vk::descriptor_set& descriptor_set, vk::swapchain& swapchain, vk::pipeline_layout& pipeline_layout, vk::pipeline& pipeline, vk::queue& present_queue)
	{
		std::srand(std::time(nullptr));

		while (!glfwWindowShouldClose(window.glfw_window))
		{
			uint32_t image_view_index = 0; // TODO GET THE INDEX
			constexpr uint32_t buffer_size = 128 * 4;

			glfwPollEvents();

			vkWaitForFences(device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
			vkResetCommandBuffer(command_buffer, 0);
			vkResetFences(device, 1, &m_in_flight_fence);

			VkBufferCreateInfo buffer_create_info{};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_create_info.size = buffer_size;
			buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

			VkBuffer buffer;
			vkCreateBuffer(device, &buffer_create_info, nullptr, &buffer);

			//VkPhysicalDeviceMemoryProperties mem_properties;
			//vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

			VkMemoryAllocateInfo memory_allocate_info{};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = buffer_size;
			memory_allocate_info.memoryTypeIndex = 2; // hard coded host-visible/coherent on my machine.

			VkDeviceMemory device_buffer_memory;
			vkAllocateMemory(device, &memory_allocate_info, nullptr, &device_buffer_memory);

			VkBindBufferMemoryInfo bind_buffer_memory_info{};
			bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
			bind_buffer_memory_info.memory = device_buffer_memory;
			bind_buffer_memory_info.buffer = buffer;
			vkBindBufferMemory(device, buffer, device_buffer_memory, 0);
			
			VkBufferViewCreateInfo buffer_view_create_info{};
			buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			buffer_view_create_info.format = VK_FORMAT_R32_UINT;
			buffer_view_create_info.range = buffer_size;
			buffer_view_create_info.buffer = buffer;

			VkBufferView buffer_view;
			vkCreateBufferView(device, &buffer_view_create_info, nullptr, &buffer_view);

			//vkBindBufferMemory

			float* mem;
			vk_check(vkMapMemory(device, device_buffer_memory, 0, buffer_size, 0, reinterpret_cast<void**>(&mem)));

			for (uint32_t i = 0; i < 100; i += 1)
			{
				mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);
			}

			// Fill out memory here.
			vkUnmapMemory(device, device_buffer_memory);
			
			VkDescriptorBufferInfo descriptor_buffer_info{};
			descriptor_buffer_info.buffer = buffer;
			descriptor_buffer_info.offset = 0;
			descriptor_buffer_info.range = buffer_size;

			VkWriteDescriptorSet write_descriptor_set{};
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = descriptor_set;
			write_descriptor_set.dstBinding = 0;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

			vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);

			vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, m_image_available_semaphore, nullptr, &image_view_index);
			
			VkClearValue clear_value{};

			VkRect2D render_area{};
			render_area.extent.width = WIDTH;
			render_area.extent.height = HEIGHT;

			VkRenderingAttachmentInfo color_attachment_info{};
			color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			color_attachment_info.clearValue = clear_value;
			color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
			color_attachment_info.imageView = swapchain.image_views[image_view_index];
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

			vkBeginCommandBuffer(command_buffer, &begin_info);

			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, descriptor_set, 0, nullptr);

			VkImageMemoryBarrier barrier1{};
			barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier1.image = swapchain.images[image_view_index];
			barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier1.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier1.subresourceRange.baseArrayLayer = 0;
			barrier1.subresourceRange.baseMipLevel = 0;
			barrier1.subresourceRange.layerCount = 1;
			barrier1.subresourceRange.levelCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			//float values[3] = { static_cast<float>(center_x), static_cast<float>(center_y), static_cast<float>(zoom)};
			//vkCmdPushConstants(m_command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 12, values);
			
			vkCmdBeginRendering(command_buffer, &rendering_info);
			vkCmdDraw(command_buffer, 6, 1, 0, 0);
			vkCmdEndRendering(command_buffer);
			
			//VkSemaphoreWaitInfo 

			//VkSemaphoreWait()

			VkImageMemoryBarrier barrier2{};
			barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier2.image = swapchain.images[image_view_index];
			barrier2.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier2.subresourceRange.baseArrayLayer = 0;
			barrier2.subresourceRange.baseMipLevel = 0;
			barrier2.subresourceRange.layerCount = 1;
			barrier2.subresourceRange.levelCount = 1;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

			vkEndCommandBuffer(command_buffer);

			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			VkSubmitInfo submit_info{};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = command_buffer;
			submit_info.pWaitSemaphores = &m_image_available_semaphore;
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitDstStageMask = &wait_stage;
			submit_info.pSignalSemaphores = &m_render_finished_semaphore;
			submit_info.signalSemaphoreCount = 1;

			//submit_info.
			
			vkQueueSubmit(present_queue, 1, &submit_info, m_in_flight_fence);

			VkPresentInfoKHR present_info{};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pSwapchains = swapchain;
			present_info.swapchainCount = 1;
			present_info.pWaitSemaphores = &m_render_finished_semaphore;
			present_info.waitSemaphoreCount = 1;
			present_info.pImageIndices = &image_view_index;

			vkQueuePresentKHR(present_queue, &present_info);

			// NO NO NO
			vkDeviceWaitIdle(device);

			vkDestroyBufferView(device, buffer_view, nullptr);
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, device_buffer_memory, nullptr);

		}
	}
}
