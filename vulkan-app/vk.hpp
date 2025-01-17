#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace vk
{
	class instance {
	private:
		VkInstance handle;
	public:
		instance();
		~instance();
		operator VkInstance() const { return handle; }
	};

	class physical_device {
	private:
		VkPhysicalDevice handle;
	public:
		physical_device(vk::instance& instance);
		physical_device() = delete;
		operator VkPhysicalDevice() const { return handle; }
	};

	class surface {
	private:
		VkInstance instance_handle;
		VkSurfaceKHR handle;
	public:
		surface(vk::instance& instance, HWND window_handle);
		surface() = delete;
		~surface();
		operator VkSurfaceKHR() const { return handle; }
	};

	class device {
	private:
		VkDevice handle;
	public:
		device(vk::physical_device& physical_device, vk::surface& surface);
		device() = delete;
		~device();
		operator VkDevice() const { return handle; }
		uint32_t queue_family_index;
	};

	class queue {
	private:
		VkQueue handle;
	public:
		queue(vk::device& device, uint32_t queue_family_index);
		queue() = delete;
		operator VkQueue() const { return handle; }
	};

	class swapchain {
	private:
		VkSwapchainKHR handle;
		vk::device& m_device;
	public:
		swapchain(vk::device& device, vk::surface& surface, uint32_t width, uint32_t height);
		swapchain() = delete;
		~swapchain();
		operator VkSwapchainKHR() const { return handle; }
		operator const VkSwapchainKHR * () const { return &handle; }
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
	};

	class command_pool {
	private:
		VkCommandPool handle;
		vk::device& device;
	public:
		command_pool(vk::device& device, uint32_t queue_family_index);
		command_pool() = delete;
		~command_pool();
		operator VkCommandPool() const { return handle; }
	};

	class command_buffer {
	private:
		VkCommandBuffer handle;
		vk::device& device;
		vk::command_pool& command_pool;
	public:
		command_buffer(vk::device& device, vk::command_pool& command_pool);
		command_buffer() = delete;
		~command_buffer();
		operator VkCommandBuffer() const { return handle; }
		operator const VkCommandBuffer*() const { return &handle; }
	};

	class shader_module {
	private:
		VkShaderModule handle;
		vk::device& device;
	public:
		shader_module(vk::device& device, std::string filename);
		shader_module() = delete;
		~shader_module();
		operator VkShaderModule() const { return handle; }
	};

	class descriptor_pool {
	private:
		VkDescriptorPool handle;
		vk::device& device;
	public:
		descriptor_pool(vk::device& device);
		descriptor_pool() = delete;
		~descriptor_pool();
		operator VkDescriptorPool() const { return handle; }
	};

	class descriptor_set_layout {
	private:
		VkDescriptorSetLayout handle;
		vk::device& device;
	public:
		descriptor_set_layout(vk::device& device);
		descriptor_set_layout() = delete;
		~descriptor_set_layout();
		operator VkDescriptorSetLayout() const { return handle; }
		operator const VkDescriptorSetLayout* () const { return &handle; }
	};

	class descriptor_set {
	private:
		VkDescriptorSet handle;
	public:
		descriptor_set(vk::device& device, vk::descriptor_pool& pool, vk::descriptor_set_layout& layout);
		descriptor_set() = delete;
		~descriptor_set();
		operator VkDescriptorSet() const { return handle; }
		operator const VkDescriptorSet*() const { return &handle; }

	};

	class pipeline_layout {
	private:
		VkPipelineLayout handle;
		vk::device& device;
	public:
		pipeline_layout(vk::device& device, vk::descriptor_set_layout& descriptor_set_layout);
		pipeline_layout() = delete;
		~pipeline_layout();
		operator VkPipelineLayout() const { return handle; }
		operator const VkPipelineLayout* () const { return &handle; }
	};


	class pipeline {
	private:
		VkPipeline handle;
		vk::device& device;
	public:
		pipeline(vk::device& device, vk::pipeline_layout& pipeline_layout, vk::shader_module& vertex_shader, vk::shader_module& fragment_shader, uint32_t width, uint32_t height);
		pipeline() = delete;
		~pipeline();
		operator VkPipeline() const { return handle; }
		operator const VkPipeline* () const { return &handle; }

	};
}
