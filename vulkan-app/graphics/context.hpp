#pragma once

#include <vector>

#include "vk.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace graphics
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

	//struct framebuffer
	//{
	//	VkImage image;
	//	VkImageView view;
	//	uint32_t index;
	//};

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

		uint32_t memory_type_device_local;
		uint32_t memory_type_host_coherent;

		std::vector<VkDeviceMemory> allocated_device_memory;

		graphics::queue transfer_queue;
		graphics::queue graphics_queue;
		graphics::queue present_queue;

		context(HWND window_handle, uint32_t width, uint32_t height);
		context() = delete;
		//context(context&&) = delete;
		~context();

		//graphics::framebuffer& get_next_framebuffer();
		void begin_command_buffer(VkCommandBuffer command_buffer);
		void begin_rendering(VkCommandBuffer command_buffer, VkImageView view, VkImageView depth_view);
		void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout) const;
		//void present(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, graphics::framebuffer& current_framebuffer);
		void submit(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_stage, VkSemaphore signal_semaphore, VkFence fence);
		void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);

		//void advance_swapchain();
		//void prepare_swapchain_for_writing(VkCommandBuffer command_buffer);
		//void prepare_swapchain_for_presentation(VkCommandBuffer command_buffer);
	private:
		//std::vector<framebuffer> framebuffers;
		//framebuffer current_framebuffer;
		void create_instance();
		void get_physical_device();
		void create_surface(HWND window_handle);
		void create_device();
		void create_descriptor_pool();
		//void create_swapchain(uint32_t width, uint32_t height);
		void create_descriptor_set_layout();
		void create_descriptor_set();
		void create_pipeline_layout();
		void create_fragment_shader();
		void create_vertex_shader();
		void create_pipeline();
	};

}
