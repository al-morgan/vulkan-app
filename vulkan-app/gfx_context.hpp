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

		uint32_t memory_type_device_local;
		uint32_t memory_type_host_coherent;

		std::vector<VkDeviceMemory> allocated_device_memory;
		
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
		void transition_image(VkCommandBuffer command_buffer, VkImage image, VkShaderStageFlags source_stage, VkAccessFlags source_access_mask, VkShaderStageFlags desintation_stage, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout);
		void present(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, uint32_t image_index);
		void submit(VkCommandBuffer command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_stage, VkSemaphore signal_semaphore, VkFence fence);
		void upload_buffer(VkBuffer buffer, void* source, VkDeviceSize buffer_size);
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

	//class buffer
	//{
	//private:
	//	VkBuffer handle;
	//	std::vector<std::byte> memory;
	//	size_t size;
	//public:
	//	void* data()
	//	{
	//		return memory.data();
	//	}

	//	buffer(const gfx::context& context, size_t size)
	//	{
	//		VkDeviceMemory device_memory;

	//		VkMemoryAllocateInfo memory_allocate_info{};
	//		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	//		memory_allocate_info.allocationSize = size;
	//		memory_allocate_info.memoryTypeIndex = context.memory_type_host_coherent;
	//		check(vkAllocateMemory(context.device, &memory_allocate_info, nullptr, &device_memory));

	//		VkBindBufferMemoryInfo bind_buffer_memory_info{};
	//		bind_buffer_memory_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
	//		bind_buffer_memory_info.memory = device_memory;
	//		bind_buffer_memory_info.buffer = handle;
	//		check(vkBindBufferMemory(context.device, handle, device_memory, 0));

	//		void* mem;
	//		check(vkMapMemory(context.device, device_memory, 0, size, 0, &mem));
	//		memcpy(mem, source, buffer_size);
	//		vkUnmapMemory(device, device_memory);
	//	}
	//};
}
