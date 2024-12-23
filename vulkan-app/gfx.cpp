#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "gfx.hpp"
#include <vector>

#define WIDTH	800
#define HEIGHT	600

namespace app
{
	static void vk_check(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan error!");
		}
	}

	app::gfx::gfx()
	{
		set_up_glfw();
		set_up_instance();
		pick_physical_device();
		set_up_device();
	}

	app::gfx::~gfx()
	{
		tear_down_device();
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

		uint32_t instance_extension_count;
		std::vector<VkExtensionProperties> instance_extensions;
		vk_check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
		instance_extensions.resize(instance_extension_count);
		vk_check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

		uint32_t instance_layer_count;
		std::vector<VkLayerProperties> instance_layers;
		vk_check(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
		instance_layers.resize(instance_layer_count);
		vk_check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

		std::vector<const char*> enabled_layers = { "VK_LAYER_KHRONOS_validation" };

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.ppEnabledExtensionNames = glfw_extensions;
		create_info.enabledExtensionCount = glfw_extension_count;
		create_info.enabledLayerCount = enabled_layers.size();
		create_info.ppEnabledLayerNames = enabled_layers.data();
		vk_check(vkCreateInstance(&create_info, nullptr, &m_instance));
	}

	void app::gfx::tear_down_instance()
	{
		vkDestroyInstance(m_instance, nullptr);
	}

	void app::gfx::pick_physical_device()
	{
		uint32_t physical_device_count;
		std::vector<VkPhysicalDevice> physical_devices;
		vk_check(vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr));
		physical_devices.resize(physical_device_count);
		vk_check(vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data()));

		if (physical_device_count != 1)
		{
			throw std::runtime_error("Multiple physical devices not supported!");
		}

		// I only have one physical device right now so I'm going to cheat
		m_physical_device = physical_devices[0];
	}

	void app::gfx::set_up_device()
	{
		uint32_t queue_family_count;
		std::vector<VkQueueFamilyProperties> queue_families;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
		queue_families.resize(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());
		uint32_t queue_family_index;

		for(uint32_t i = 0; i < queue_family_count; i++)
		{
			constexpr VkFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
			if ((queue_families[i].queueFlags & required_flags) == required_flags)
			{
				queue_family_index = i;
				break;
			}
		}

		float queue_priority = 1.0f;

		//VkQueueCreateInfo queue_create_info{};
		//queue_

		VkDeviceQueueCreateInfo device_queue_create_info{};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.queueCount = 1;
		device_queue_create_info.queueFamilyIndex = queue_family_index;
		device_queue_create_info.pQueuePriorities = &queue_priority;

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = &device_queue_create_info;
		create_info.queueCreateInfoCount = 1;
		vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device);
	}

	void app::gfx::tear_down_device()
	{
		vkDestroyDevice(m_device, nullptr);
	}

	void app::gfx::update()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
		}
	}
	


}