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

vk::instance::instance()
{
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

	std::vector<const char*> enabled_extensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };

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

vk::instance::~instance()
{
	vkDestroyInstance(handle, nullptr);
}

vk::physical_device::physical_device(vk::instance& instance)
{
	uint32_t physical_device_count;
	std::vector<VkPhysicalDevice> physical_devices;
	check(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
	physical_devices.resize(physical_device_count);
	check(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));

	if (physical_device_count != 1)
	{
		throw std::runtime_error("Multiple physical devices not supported!");
	}

	// I only have one physical device right now so I'm going to cheat
	handle = physical_devices[0];
}


vk::surface::surface(vk::instance& instance, HWND window_handle)
{
	instance_handle = instance;
	
	VkWin32SurfaceCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_handle; // glfwGetWin32Window(m_window);
	create_info.hinstance = GetModuleHandle(nullptr);

	check(vkCreateWin32SurfaceKHR(instance_handle, &create_info, nullptr, &handle));
}

vk::surface::~surface()
{
	vkDestroySurfaceKHR(instance_handle, handle, nullptr);
}

vk::device::device(vk::physical_device& physical_device, vk::surface& surface)
{
	uint32_t queue_family_count;
	std::vector<VkQueueFamilyProperties> queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
	std::optional<uint32_t> queue_family_index_o;

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		constexpr VkFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
		VkBool32 surface_support = VK_FALSE;

		check(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &surface_support));
		if ((queue_families[i].queueFlags & required_flags) == required_flags && surface_support)
		{
			queue_family_index_o = i;
			break;
		}
	}

	if (!queue_family_index_o.has_value())
	{
		throw std::runtime_error("Queue selection failed!");
	}

	queue_family_index = queue_family_index_o.value();

	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.queueFamilyIndex = queue_family_index;
	device_queue_create_info.pQueuePriorities = &queue_priority;

	std::vector<const char*> enabled_extensions = { "VK_KHR_swapchain", "VK_KHR_dynamic_rendering" };

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = &dynamic_rendering_features;
	create_info.pQueueCreateInfos = &device_queue_create_info;
	create_info.queueCreateInfoCount = 1;
	create_info.ppEnabledExtensionNames = enabled_extensions.data();
	create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());

	vkCreateDevice(physical_device, &create_info, nullptr, &handle);
}

vk::device::~device()
{
	vkDestroyDevice(handle, nullptr);
}

vk::queue::queue(vk::device& device, uint32_t queue_family_index)
{
	vkGetDeviceQueue(device, queue_family_index, 0, &handle);
}


// VkInstance handle;
//};
//}
