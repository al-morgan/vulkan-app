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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "file.hpp"
#include "vk.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "gfx_context.hpp"


//#include <vulkan/vulkan_win32.h>

#define WIDTH	800
#define HEIGHT	800

static double mouse_x, mouse_y;
static double mouse_down_x, mouse_down_y;
static bool is_mouse_down;
static double center_x, center_y;
static double last_update_x, last_update_y;
static double zoom = 1.0;

//namespace app
//{

struct vertex
{
	glm::mat3 pos;
};

struct mvp
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

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

	app::engine::engine(gfx::context& context) : context(context)
	{
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(context.device, &fence_create_info, nullptr, &m_in_flight_fence);

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		//vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_image_available_semaphore);
		vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);	}

	app::engine::~engine()
	{
		vkDeviceWaitIdle(context.device);

		vkDestroySemaphore(context.device, m_render_finished_semaphore, nullptr);
		//vkDestroySemaphore(context.device, m_image_available_semaphore, nullptr);
		vkDestroyFence(context.device, m_in_flight_fence, nullptr);
	}

	void app::engine::update(gfx::context& context, app::window& window, vk::command_buffer& command_buffer)
	{
		std::srand(std::time(nullptr));

		#define MIN -1.0
		#define MAX 1.0

		vk::Vertex positions[6] =
		{
			glm::vec2(MIN, MIN),
			glm::vec2(MAX, MIN),
			glm::vec2(MIN, MAX),
			glm::vec2(MAX, MIN),
			glm::vec2(MAX, MAX),
			glm::vec2(MIN, MAX)
		};

		VkBuffer vertex_buffer;
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = sizeof(positions);
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vk_check(vkCreateBuffer(context.device, &buffer_create_info, nullptr, &vertex_buffer));

		context.upload_buffer(vertex_buffer, positions, sizeof(positions));

		gfx::buffer vbuffer(context, 112 * 12 * 3 * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		gfx::buffer ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		while (!glfwWindowShouldClose(window.glfw_window))
		{
			//uint32_t image_view_index = 0; // TODO GET THE INDEX
			constexpr uint32_t buffer_size = 128 * 4;

			glfwPollEvents();
			vkWaitForFences(context.device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
			vkResetCommandBuffer(command_buffer, 0);
			vkResetFences(context.device, 1, &m_in_flight_fence);

			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_create_info.size = buffer_size;
			buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

			
			VkBuffer buffer;
			vkCreateBuffer(context.device, &buffer_create_info, nullptr, &buffer);


			VkMemoryAllocateInfo memory_allocate_info{};
			memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocate_info.allocationSize = buffer_size;
			memory_allocate_info.memoryTypeIndex = context.memory_type_host_coherent;

			VkDeviceMemory device_buffer_memory;
			vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &device_buffer_memory);
			
			VkBindBufferMemoryInfo bind_buffer_memory_info{};
			bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
			bind_buffer_memory_info.memory = device_buffer_memory;
			bind_buffer_memory_info.buffer = buffer;
			vkBindBufferMemory(context.device, buffer, device_buffer_memory, 0);
			
			VkBufferViewCreateInfo buffer_view_create_info{};
			buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			buffer_view_create_info.format = VK_FORMAT_R32_UINT;
			buffer_view_create_info.range = buffer_size;
			buffer_view_create_info.buffer = buffer;

			VkBufferView buffer_view;
			vkCreateBufferView(context.device, &buffer_view_create_info, nullptr, &buffer_view);

			//vkBindBufferMemory

			float* mem;
			vk_check(vkMapMemory(context.device, device_buffer_memory, 0, buffer_size, 0, reinterpret_cast<void**>(&mem)));

			int x = 0;
			int y = 0;
			int j = 0;
			for (uint32_t i = 0; i < 100; i += 1)
			{
				mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);

				float z = static_cast<float>(std::rand()) * .1f / static_cast<float>(RAND_MAX);
				
				if (x != 10 && y != 10)
				{
					glm::vec3* foo = static_cast<glm::vec3*>(vbuffer.data());
					foo[j][0] = static_cast<float>(x * .2f - 1.f);
					foo[j][1] = static_cast<float>(y * .2f - 1.f);
					foo[j][2] = z;
					j++;

					foo[j][0] = static_cast<float>(x + 1) * .2f - 1.f;
					foo[j][1] = static_cast<float>(y) * .2f - 1.f;
					foo[j][2] = z;
					j++;

					foo[j][0] = static_cast<float>(x) * .2f - 1.f;
					foo[j][1] = static_cast<float>(y + 1) * .2f - 1.f;
					foo[j][2] = z;
					j++;

					foo[j][0] = static_cast<float>((x + 1) * .2f - 1.f);
					foo[j][1] = static_cast<float>(y) * .2f - 1.f;
					foo[j][2] = z;
					j++;

					foo[j][0] = static_cast<float>(x + 1) * .2f - 1.f;
					foo[j][1] = static_cast<float>(y + 1) * .2f - 1.f;
					foo[j][2] = z;
					j++;

					foo[j][0] = static_cast<float>(x * .2f - 1.f);
					foo[j][1] = static_cast<float>((y + 1) * .2f - 1.f);
					foo[j][2] = z;
					j++;
				}
				
				x += 1;
				if (x == 10)
				{
					x = 0;
					y++;
				}

			}

			//vbuffer.update(context, command_buffer);

			// Fill out memory here.
			vkUnmapMemory(context.device, device_buffer_memory);

			//vk_check(vkMapMemory(context.device, vertex_buffer_memory, 0, sizeof(positions), 0, reinterpret_cast<void**>(&mem)));

			//memcpy(mem, positions, sizeof(positions));
			//vkUnmapMemory(context.device, vertex_buffer_memory);


			//vkBindBufferMemory(context.device, vertex_buffer, vertex_buffer_memory, 0);
			
			VkDescriptorBufferInfo descriptor_buffer_info{};
			descriptor_buffer_info.buffer = buffer;
			descriptor_buffer_info.offset = 0;
			descriptor_buffer_info.range = buffer_size;

			VkWriteDescriptorSet write_descriptor_set{};
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = context.descriptor_set;
			write_descriptor_set.dstBinding = 0;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

			vkUpdateDescriptorSets(context.device, 1, &write_descriptor_set, 0, nullptr);

			mvp* ubo = static_cast<mvp*>(ubuffer.data());
			ubo->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
			ubo->model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
			ubo->proj = glm::perspective(glm::radians(45.0f),
				800.0f / 800.0f, 0.1f,
				10.0f);

			descriptor_buffer_info.buffer = ubuffer.destination;
			descriptor_buffer_info.offset = 0;
			descriptor_buffer_info.range = ubuffer.m_size;

			write_descriptor_set.dstBinding = 1;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

			vkUpdateDescriptorSets(context.device, 1, &write_descriptor_set, 0, nullptr);

			gfx::framebuffer &framebuffer = context.get_next_framebuffer();			
			context.begin_command_buffer(command_buffer);

			ubuffer.update(context, command_buffer);
			vbuffer.update(context, command_buffer);

			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline_layout, 0, 1, &context.descriptor_set, 0, nullptr);
						
			context.transition_image(
				command_buffer,
				framebuffer.image,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_ACCESS_NONE,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vbuffer.destination, &offset);

			//float values[3] = { static_cast<float>(center_x), static_cast<float>(center_y), static_cast<float>(zoom)};
			//vkCmdPushConstants(m_command_buffer, m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 12, values);
			
			context.begin_rendering(command_buffer, framebuffer.view);
			vkCmdDraw(command_buffer, 600, 1, 0, 0);
			vkCmdEndRendering(command_buffer);
			
			context.transition_image(
				command_buffer,
				framebuffer.image,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_NONE,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);

			vkEndCommandBuffer(command_buffer);

			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			context.submit(
				command_buffer,
				context.get_next_framebuffer_semaphore,
				wait_stage,
				m_render_finished_semaphore,
				m_in_flight_fence);

			context.present(command_buffer, m_render_finished_semaphore, framebuffer.index);

			// NO NO NO
			vkDeviceWaitIdle(context.device);

			vkDestroyBufferView(context.device, buffer_view, nullptr);
			vkDestroyBuffer(context.device, buffer, nullptr);
			vkFreeMemory(context.device, device_buffer_memory, nullptr);
			//vkFreeMemory(context.device, vertex_buffer_memory, nullptr);
		}
		
		vkDestroyBuffer(context.device, vertex_buffer, nullptr);
	}
//}
