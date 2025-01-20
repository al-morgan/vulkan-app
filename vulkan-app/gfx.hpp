#pragma once

#include <vector>
#include "gfx_context.hpp"

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>



namespace app
{
	class window
	{
	private:
	public:
		window();
		~window();
		GLFWwindow* glfw_window;
		HWND handle;
	};

	class engine
	{
	private:
		gfx::context& context;
		VkFence m_in_flight_fence;
		//VkSemaphore m_image_available_semaphore;
		VkSemaphore m_render_finished_semaphore;

	public:
		engine(gfx::context& context);
		~engine();
		void update(gfx::context& context, app::window& window, vk::command_buffer& command_buffer, vk::descriptor_set& descriptor_set, vk::pipeline_layout& pipeline_layout, vk::pipeline& pipeline);
	};
}
