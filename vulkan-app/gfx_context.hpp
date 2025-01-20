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
	struct queue
	{
	public:
		VkQueue handle;
		uint32_t family_index;
	};

	struct framebuffer
	{
		VkImage image;
		VkImageView view;
	};

	/// <summary>
	/// Holds everything that gets set up once per application.
	/// </summary>
	class context
	{
	public:
		std::vector<framebuffer> framebuffers;
		VkInstance	instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkSemaphore image_available_semaphore;
		
		gfx::queue transfer_queue;
		gfx::queue graphics_queue;
		gfx::queue present_queue;


		context(HWND window_handle, uint32_t width, uint32_t height);
		context() = delete;
		context(context&&) = delete;
		~context();

		gfx::framebuffer& get_next_framebuffer();
	private:
		void create_instance();
		void get_physical_device();
		void create_surface(HWND window_handle);
		void create_device();
		void create_swapchain(uint32_t width, uint32_t height);
	};
}

