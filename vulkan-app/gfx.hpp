#pragma once

#include <vector>


#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "graphics/context.hpp"


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
		graphics::context& context;
		VkFence m_in_flight_fence;
		VkSemaphore m_render_finished_semaphore;

	public:
		engine(graphics::context& context);
		~engine();
		void update(graphics::context& context, app::window& window);
	};
}
