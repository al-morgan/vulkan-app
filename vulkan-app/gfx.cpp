#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "gfx.hpp"
#include <vector>
#include <limits>
#include <optional>

//#include <vulkan/vulkan_win32.h>

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
		set_up_surface();
		set_up_device();
		set_up_queues();
		set_up_swap_chain();
	}

	app::gfx::~gfx()
	{
		tear_down_swap_chain();
		tear_down_queues();
		tear_down_device();
		tear_down_surface();
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

		std::vector<const char*> enabled_layers = {"VK_LAYER_KHRONOS_validation"};

		//VkDebugUtilsMessen

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.ppEnabledExtensionNames = glfw_extensions;
		create_info.enabledExtensionCount = glfw_extension_count;
		create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
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
		std::optional<uint32_t> queue_family_index;

		for(uint32_t i = 0; i < queue_family_count; i++)
		{
			constexpr VkFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
			VkBool32 surface_support = VK_FALSE;
			
			vk_check(vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &surface_support));
			if ((queue_families[i].queueFlags & required_flags) == required_flags && surface_support)
			{
				queue_family_index = i;
				break;
			}
		}

		if (!queue_family_index.has_value())
		{
			throw std::runtime_error("Queue selection failed!");
		}
		
		m_queue_family_index = queue_family_index.value();

		float queue_priority = 1.0f;

		VkDeviceQueueCreateInfo device_queue_create_info{};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.queueCount = 1;
		device_queue_create_info.queueFamilyIndex = m_queue_family_index;
		device_queue_create_info.pQueuePriorities = &queue_priority;

		std::vector<const char *> enabled_extensions = {"VK_KHR_swapchain"};
		
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = &device_queue_create_info;
		create_info.queueCreateInfoCount = 1;
		create_info.ppEnabledExtensionNames = enabled_extensions.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());

		vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device);
	}

	void app::gfx::tear_down_device()
	{
		vkDestroyDevice(m_device, nullptr);
	}

	void app::gfx::set_up_surface()
	{
		VkWin32SurfaceCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = glfwGetWin32Window(m_window);
		create_info.hinstance = GetModuleHandle(nullptr);

		vk_check(vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface));
	}

	void app::gfx::tear_down_surface()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	void app::gfx::set_up_queues()
	{
		vkGetDeviceQueue(m_device, m_queue_family_index, 0, &m_graphics_queue);
		
		// Hack for now, maybe optimize later.
		m_present_queue = m_graphics_queue;
	}

	void app::gfx::tear_down_queues()
	{

	}

	void app::gfx::set_up_swap_chain()
	{
		VkExtent2D extent{};
		extent.width = WIDTH;
		extent.height = HEIGHT;
		
		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = m_surface;
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

		vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain);

		uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
		m_swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, m_swapchain_images.data());

		m_swapchain_image_views.resize(swapchain_image_count);

		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			VkImageViewCreateInfo image_view_create_info{};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = m_swapchain_images[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.subresourceRange.levelCount = 1;

			vkCreateImageView(m_device, &image_view_create_info, nullptr, &m_swapchain_image_views[i]);
		}
	}

	void app::gfx::tear_down_swap_chain()
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_swapchain_image_views.size()); i++)
		{
			vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
		}
		
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	}

	void app::gfx::update()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
		}
	}
}
