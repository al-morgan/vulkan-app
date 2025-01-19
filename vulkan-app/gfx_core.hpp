#pragma once

#include <vector>

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace gfx
{
	class core
	{
	public:
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkQueue queue;
		uint32_t queue_family_index;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		core(HWND window_handle, uint32_t width, uint32_t height);
		core() = delete;
		~core();
	private:
		void create_instance();
		void get_physical_device();
		void create_surface(HWND window_handle);
		void create_device();
		void create_swapchain(uint32_t width, uint32_t height);
	};
}
