#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "gfx.hpp"

#define WIDTH	800
#define HEIGHT	600

namespace app
{
	app::gfx::gfx()
	{
		set_up_glfw();
	}

	app::gfx::~gfx()
	{
		tear_down_glfw();
	}

	void app::gfx::update()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
		}
	}
	
	void app::gfx::set_up_glfw()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void app::gfx::tear_down_glfw()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
}
