#pragma once

#include <vector>

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace gfx
{
	/// <summary>
	/// A device queue.
	/// </summary>
	class queue
	{
	public:
		VkQueue handle;
		uint32_t family_index;
	};


	class framebuffer
	{
	public:
		VkImage image;
		VkImageView view;
	};

	class swapchain
	{
	public:
		VkSwapchainKHR handle;
		std::vector<framebuffer> framebuffers;
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
		//VkSwapchainKHR swapchain;
		
		gfx::queue transfer_queue;
		gfx::queue graphics_queue;
		gfx::queue present_queue;

		gfx::swapchain swapchain;
		
		//VkQueue queue;
		//uint32_t queue_family_index;


		//std::vector<VkImageView> image_views;
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

