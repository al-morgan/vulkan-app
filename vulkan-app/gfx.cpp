#include <vulkan/vulkan.h>

#include "gfx.hpp"

#define WIDTH	800
#define HEIGHT	600

namespace app
{
	void app::gfx::init_instance()
	{
		int foo;
	}

	app::gfx::gfx()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}
	
	void app::gfx::init_window()
	{
	}
}
