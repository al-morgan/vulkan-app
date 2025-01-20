#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>
#include <optional>

#include "file.hpp"
#include "vk.hpp"
#include "gfx_context.hpp"

static void check(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan error!");
	}
}

/// <summary>
/// Set up the Vulkan instance
/// </summary>
void gfx::context::create_instance()
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

	uint32_t instance_extension_count = 0;
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
		
	check(vkCreateInstance(&create_info, nullptr, &instance));
}

/// <summary>
/// Get the Vulkan physical device
/// </summary>
void gfx::context::get_physical_device()
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
	physical_device = physical_devices[0];
}

/// <summary>
/// Create the surface
/// </summary>
/// <param name="window_handle"></param>
void gfx::context::create_surface(HWND window_handle)
{
	VkWin32SurfaceCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hwnd = window_handle;
	create_info.hinstance = GetModuleHandle(nullptr);

	check(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));	
}

/// <summary>
/// Create the logical device.
/// </summary>
void gfx::context::create_device()
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

	graphics_queue.family_index = queue_family_index_o.value();

	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.queueFamilyIndex = graphics_queue.family_index;
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

	vkCreateDevice(physical_device, &create_info, nullptr, &device);
}

/// <summary>
/// Create the swapchain
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
void gfx::context::create_swapchain(uint32_t width, uint32_t height)
{
	VkExtent2D extent{};
	extent.width = width;
	extent.height = height;

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;
	create_info.minImageCount = 3;
	create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);

	uint32_t swapchain_image_count;
	std::vector<VkImage> images;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
	images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, images.data());

	framebuffers.resize(swapchain_image_count);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		framebuffers[i].image = images[i];

		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = framebuffers[i].image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.levelCount = 1;

		vkCreateImageView(device, &image_view_create_info, nullptr, &framebuffers[i].view);
	}
}

/// <summary>
/// Initialize the context
/// </summary>
/// <param name="window_handle"></param>
/// <param name="width"></param>
/// <param name="height"></param>
gfx::context::context(HWND window_handle, uint32_t width, uint32_t height)
{
	create_instance();
	create_surface(window_handle);
	get_physical_device();
	create_device();
	vkGetDeviceQueue(device, graphics_queue.family_index, 0, &graphics_queue.handle);
	create_swapchain(width, height);
}

/// <summary>
/// Destroy the context
/// </summary>
gfx::context::~context()
{
	for (uint32_t i = 0; i < static_cast<uint32_t>(framebuffers.size()); i++)
	{
		vkDestroyImageView(device, framebuffers[i].view, nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}
