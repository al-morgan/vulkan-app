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

#include "graphics/descriptor_set.hpp"
#include "graphics/graphics.hpp"
#include "graphics/context.hpp"
#include "graphics/buffer.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/image.hpp"

#include "perlin.hpp"

#define WIDTH	800
#define HEIGHT	800

static double mouse_x, mouse_y;
static double mouse_down_x, mouse_down_y;
static bool is_mouse_down;
static double center_x, center_y;
static double last_update_x, last_update_y;
static double zoom = 1.0;

static double yaw;
static double pitch;
static bool jumping;

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
		constexpr double sensitivity = 0.001;

		yaw += (xpos) * sensitivity;
		pitch += (ypos) * sensitivity;

		glfwSetCursorPos(window, 0.0, 0.0);

		//mouse_x = xpos;
		//mouse_y = ypos;

		//std::cout << xpos << ", " << ypos << std::endl;
		
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
		}
	}

	static int keys[GLFW_KEY_LAST];

	static void key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		keys[key] = action;
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
		glfwSetKeyCallback(glfw_window, key_press_callback);

		glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			throw std::runtime_error("Could not set mouse to raw input mode!");
		}

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

	app::engine::engine(graphics::context& context) : context(context)
	{
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(context.device, &fence_create_info, nullptr, &m_in_flight_fence);

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(context.device, &semaphore_create_info, nullptr, &m_render_finished_semaphore);
	}

	app::engine::~engine()
	{
		vkDeviceWaitIdle(context.device);

		vkDestroySemaphore(context.device, m_render_finished_semaphore, nullptr);
		vkDestroyFence(context.device, m_in_flight_fence, nullptr);
	}

	void app::engine::update(graphics::context& context, app::window& window, vk::command_buffer& command_buffer)
	{
		std::srand(std::time(nullptr));

		#define MIN -1.0
		#define MAX 1.0

		graphics::vertex2d positions[6] =
		{
			glm::vec2(MIN, MIN),
			glm::vec2(MAX, MIN),
			glm::vec2(MIN, MAX),
			glm::vec2(MAX, MIN),
			glm::vec2(MAX, MAX),
			glm::vec2(MIN, MAX)
		};

		app::perlin noise(10, 10);

		graphics::buffer vbuffer(context, 112 * 12 * 3 * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		graphics::buffer ubuffer(context, sizeof(mvp), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		constexpr int axis_size = 1000;

		static graphics::vertex3d mesh[axis_size + 1][axis_size + 1];

		for (int x = 0; x <= axis_size; x++)
		{
			for (int y = 0; y <= axis_size; y++)
			{
				mesh[y][x].pos[0] = static_cast<float>(x) / static_cast<float>(axis_size);
				mesh[y][x].pos[1] = static_cast<float>(y) / static_cast<float>(axis_size);
				//mesh[y][x].pos[2] = static_cast<float>(std::rand()) * .01f / static_cast<float>(RAND_MAX);
				mesh[y][x].pos[2] = noise.get(mesh[y][x].pos[0], mesh[y][x].pos[1]) * 0.06f;
			}
		}

		std::vector<graphics::vertex3d> points;

		for (int x = 0; x < axis_size; x++)
		{
			for (int y = 0; y < axis_size; y++)
			{
				points.push_back(mesh[y][x]);
				points.push_back(mesh[y][x + 1]);
				points.push_back(mesh[y + 1][x]);
				points.push_back(mesh[y][x + 1]);
				points.push_back(mesh[y + 1][x + 1]);
				points.push_back(mesh[y + 1][x]);
			}
		}

		int foo = points.size();

		graphics::swapchain swapchain(context, WIDTH, HEIGHT, context.surface);

		graphics::vertex3d bar = mesh[900][900];

		graphics::buffer new_vertex_buffer(context, points.size() * sizeof(graphics::vertex3d), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		memcpy(new_vertex_buffer.data(), points.data(), points.size() * sizeof(graphics::vertex3d));

		graphics::image depth_buffer(context, WIDTH, HEIGHT, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, false);

		glm::vec3 position(.20f, .20f, .20f);
		glm::vec3 direction(1.0f, 0.0f, 0.0f);

		double fall_speed = 0.0;

		graphics::buffer rbuffer(context, 128 * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

		float* mem = static_cast<float*>(rbuffer.data());

		//vkBindBufferMemory

		int x = 0;
		int y = 0;
		int j = 0;
		for (uint32_t i = 0; i < 100; i += 1)
		{
			mem[i] = static_cast<float>(std::rand()) * 2.0f * 3.14159f / static_cast<float>(RAND_MAX);

			float z = static_cast<float>(std::rand()) * .01f / static_cast<float>(RAND_MAX);

			z = 0.0f; // mem[i] / 600.f;

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

		graphics::descriptor_set the_set(context);
		the_set.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
		the_set.add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		the_set.commit();



		while (!glfwWindowShouldClose(window.glfw_window))
		{
			//uint32_t image_view_index = 0; // TODO GET THE INDEX
			constexpr uint32_t buffer_size = 128 * 4;

			glfwPollEvents();
			vkWaitForFences(context.device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
			vkResetCommandBuffer(command_buffer, 0);
			vkResetFences(context.device, 1, &m_in_flight_fence);

			VkDescriptorBufferInfo descriptor_buffer_info{};
			descriptor_buffer_info.buffer = rbuffer.handle();
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
			//ubo->view = glm::lookAt(glm::vec3(.20f, .20f, .20f), glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			//position[0] += direction[0] * 0.001f;
			//position[1] += direction[1] * 0.001f;
			
			direction[0] = sin(yaw);
			direction[1] = cos(yaw);
			direction[2] = -sin(pitch);

			glm::vec3 left(sin(yaw - 1.57), cos(yaw - 1.57), -sin(pitch));			
			
			double floor = noise.get(position[0], position[1]) * 0.06f + 0.04f;

			if (position[2] < floor)
			{
				position[2] = floor;
				jumping = false;
			}
			else
			{
				fall_speed += .0001f;
			}

			if (keys[GLFW_KEY_W] != GLFW_RELEASE)
			{
				position += direction * 0.001f;
			}

			if (keys[GLFW_KEY_A] != GLFW_RELEASE)
			{
				position += left * 0.001f;
			}

			if (keys[GLFW_KEY_S] != GLFW_RELEASE)
			{
				position -= direction * 0.001f;
			}

			if (keys[GLFW_KEY_D] != GLFW_RELEASE)
			{
				position -= left * 0.001f;
			}

			if (keys[GLFW_KEY_SPACE] != GLFW_RELEASE)
			{
				if (!jumping)
				{
					fall_speed -= .01f;
				}
				jumping = true;
				
			}

			//std::cout << keys[GLFW_KEY_W] << std::endl;

			position[2] -= fall_speed;


			

			
			ubo->view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 0.0f, 1.0f));

			ubo->model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));

			ubo->model = glm::mat4(1);/*  glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));*/

			ubo->proj = glm::perspective(glm::radians(45.0f),
				800.0f / 800.0f, 0.001f,
				10.0f);

			ubo->proj[1][1] *= -1.0f;

			descriptor_buffer_info.buffer = ubuffer.handle();
			descriptor_buffer_info.offset = 0;
			descriptor_buffer_info.range = ubuffer.size();

			write_descriptor_set.dstBinding = 1;
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
			
			vkUpdateDescriptorSets(context.device, 1, &write_descriptor_set, 0, nullptr);

			swapchain.get_next_framebuffer();
			
			context.begin_command_buffer(command_buffer);

			ubuffer.copy(command_buffer);
			vbuffer.copy(command_buffer);
			rbuffer.copy(command_buffer);
			new_vertex_buffer.copy(command_buffer);

			depth_buffer.transition(command_buffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline_layout, 0, 1, &context.descriptor_set, 0, nullptr);
			swapchain.prepare_swapchain_for_writing(command_buffer);
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

			VkDeviceSize offset = 0;
			VkBuffer buffers[] = { new_vertex_buffer.handle() };
			vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, &offset);

			VkMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);

			context.begin_rendering(command_buffer, swapchain.image_view(), depth_buffer.view());
			vkCmdDraw(command_buffer, 6000000, 1, 0, 0);
			vkCmdEndRendering(command_buffer);
			
			swapchain.prepare_swapchain_for_presentation(command_buffer);

			vkEndCommandBuffer(command_buffer);

			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			context.submit(
				command_buffer,
				swapchain.get_semaphore(),
				wait_stage,
				m_render_finished_semaphore,
				m_in_flight_fence);

			swapchain.present(m_render_finished_semaphore);

			// NO NO NO
			vkDeviceWaitIdle(context.device);
		}
		
	}
//}
