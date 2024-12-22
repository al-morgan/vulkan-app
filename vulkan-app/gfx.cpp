#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "gfx.hpp"
#include <vector>

#define WIDTH	800
#define HEIGHT	600

namespace app
{
	app::gfx::gfx()
	{
		set_up_glfw();
		set_up_instance();
	}

	app::gfx::~gfx()
	{
		tear_down_instance();
		tear_down_glfw();
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

	void app::gfx::set_up_instance()
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Vulcan App";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Unknown Engine.";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_3;

		uint32_t glfw_extension_count;
		const char** glfw_extensions;
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		VkInstanceCreateInfo create_info{};
		create_info.pApplicationInfo = &app_info;
		create_info.ppEnabledExtensionNames = glfw_extensions;
		create_info.enabledExtensionCount = glfw_extension_count;

		uint32_t instance_extensions_count;
		std::vector<VkExtensionProperties> instance_extensions;
		
		vkEnumerateInstanceExtensionProperties(nullptr, &instance_extensions_count, nullptr);
		instance_extensions.resize(instance_extensions_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &instance_extensions_count, instance_extensions.data());

		for (auto& extension : instance_extensions)
		{
			std::cout << extension.extensionName << std::endl;
		}

		std::cout << std::endl;

		for (uint32_t i = 0; i < glfw_extension_count; i++)
		{
			std::cout << glfw_extensions[i] << std::endl;
		}


		VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
	}

	void app::gfx::tear_down_instance()
	{
		vkDestroyInstance(m_instance, nullptr);
	}

	void app::gfx::update()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
		}
	}
	

}
