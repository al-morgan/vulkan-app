#pragma once

#include <vector>

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace gfx
{
	class queue
	{
	public:
		VkQueue handle;
		uint32_t family_index;
	};
	
	/// <summary>
	/// Holds everything that gets set up once per application.
	/// </summary>
	class context
	{
	public:
		VkInstance	instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physical_device;
		VkDevice device;		
		VkSwapchainKHR swapchain;
		
		gfx::queue transfer_queue;
		gfx::queue graphics_queue;
		gfx::queue present_queue;
		
		//VkQueue queue;
		//uint32_t queue_family_index;


		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		context(HWND window_handle, uint32_t width, uint32_t height);
		context() = delete;
		context(context&&) = delete;
		~context();
	private:
		void create_instance();
		void get_physical_device();
		void create_surface(HWND window_handle);
		void create_device();
		void create_swapchain(uint32_t width, uint32_t height);
	};
}

