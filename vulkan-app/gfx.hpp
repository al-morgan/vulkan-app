#pragma once

#include <vector>

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

	class gfx
	{
	private:
		vk::device& device;
		VkFence m_in_flight_fence;
		VkSemaphore m_image_available_semaphore;
		VkSemaphore m_render_finished_semaphore;

	public:
		gfx(vk::device& device);
		~gfx();
		void update(app::window& window, vk::device& device, vk::command_buffer& command_buffer, vk::descriptor_set& descriptor_set, vk::swapchain& swapchain, vk::pipeline_layout& pipeline_layout, vk::pipeline& pipeline, vk::queue& present_queue);
	};
}
