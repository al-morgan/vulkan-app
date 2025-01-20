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
		uint32_t index;
	};

	/// <summary>
	/// Holds everything that gets set up once per application.
	/// </summary>
	class context
	{
	public:
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkSwapchainKHR swapchain;
		VkSemaphore get_next_framebuffer_semaphore;
		VkDescriptorPool descriptor_pool;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorSet descriptor_set;
		VkPipelineLayout pipeline_layout;
		VkPipeline pipeline;
		VkShaderModule vertex_shader;
		VkShaderModule fragment_shader;
		
		gfx::queue transfer_queue;
		gfx::queue graphics_queue;
		gfx::queue present_queue;

		context(HWND window_handle, uint32_t width, uint32_t height);
		context() = delete;
		context(context&&) = delete;
		~context();

		gfx::framebuffer& get_next_framebuffer();
		void begin_command_buffer(VkCommandBuffer command_buffer);
		void begin_rendering(VkCommandBuffer command_buffer, VkImageView image_view);
	private:
		std::vector<framebuffer> framebuffers;
		void create_instance();
		void get_physical_device();
		void create_surface(HWND window_handle);
		void create_device();
		void create_descriptor_pool();
		void create_swapchain(uint32_t width, uint32_t height);
		void create_descriptor_set_layout();
		void create_descriptor_set();
		void create_pipeline_layout();
		void create_fragment_shader();
		void create_vertex_shader();
		void create_pipeline();
	};
}

