#include <iostream>
#include <cstdlib>
#include <ctime>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <limits>
#include <optional>
#include <fstream>
#include <array>

#include "file.hpp"
#include "vk.hpp"

//#include <vulkan/vulkan_win32.h>

//namespace vk
//{
	static void check(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan error!");
		}
	}


		vk::instance::instance() {
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = "Vulcan App";
			app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.pEngineName = "Unknown Engine.";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = VK_API_VERSION_1_3;

			//uint32_t glfw_extension_count;
			//const char** glfw_extensions;
			//glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

			std::vector<const char*> enabled_extensions = { "VK_KHR_surface", "VK_KHR_win32_surface"};

			uint32_t instance_extension_count;
			std::vector<VkExtensionProperties> instance_extensions;
			check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
			instance_extensions.resize(instance_extension_count);
			check(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

			uint32_t instance_layer_count;
			std::vector<VkLayerProperties> instance_layers;
			check(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
			instance_layers.resize(instance_layer_count);
			check(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

			std::vector<const char*> enabled_layers = { "VK_LAYER_KHRONOS_validation" };

			VkInstanceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &app_info;
			create_info.ppEnabledExtensionNames = enabled_extensions.data();
			create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
			create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
			create_info.ppEnabledLayerNames = enabled_layers.data();
			check(vkCreateInstance(&create_info, nullptr, &handle));
		}

		vk::instance::~instance() {
			vkDestroyInstance(handle, nullptr);
		}

		VkInstance handle;
	//};
//}
