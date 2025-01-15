#pragma once

#include <vector>

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


namespace app
{



	class window
	{
	private:
	public:
		window();
		~window();
		GLFWwindow* glfw_window;
		HWND handle;
	};

	class gfx
	{
	private:
		//GLFWwindow* m_window;
		// VkInstance m_instance;

		app::window m_window;
		vk::instance m_instance;
		vk::physical_device m_physical_device;
		vk::surface m_surface;
		vk::device m_device;
		vk::queue m_present_queue;
		vk::queue m_graphics_queue;
		vk::swapchain m_swapchain;
		vk::command_pool m_command_pool;
		vk::command_buffer m_command_buffer;

		//VkPhysicalDevice m_physical_device;
		//VkDevice m_device;
		//VkSurfaceKHR m_surface;
		//VkQueue m_graphics_queue;
		//VkQueue m_present_queue;
		uint32_t m_queue_family_index;	// Used for both graphics and present for now.
		//VkSwapchainKHR m_swapchain;
		//std::vector<VkImage> m_swapchain_images;
		//std::vector<VkImageView> m_swapchain_image_views;
		//VkCommandPool m_command_pool;
		VkShaderModule m_fragment_shader_module;
		VkShaderModule m_vertex_shader_module;
		VkDescriptorPool m_descriptor_pool;
		VkPipeline m_pipeline;
		//VkCommandBuffer m_command_buffer;

		VkPipelineLayout m_pipeline_layout;
		VkDescriptorSet m_descriptor_set;
		VkDescriptorSetLayout m_layout;


		VkFence m_in_flight_fence;
		VkSemaphore m_image_available_semaphore;
		VkSemaphore m_render_finished_semaphore;

		void set_up_command_pool();
		void set_up_shaders();
		void tear_down_shaders();
		void set_up_descriptor_pool();
		void tear_down_descriptor_pool();
		void set_up_pipeline();
		void tear_down_pipeline();

	public:
		gfx();
		~gfx();
		void update();
	};
}
