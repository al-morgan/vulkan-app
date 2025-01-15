#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>

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
		operator VkPhysicalDevice() const { return handle; }
	};

	class surface {
	private:
		VkInstance instance_handle;
		VkSurfaceKHR handle;
	public:
		surface(vk::instance& instance, HWND window_handle);
		~surface();
		operator VkSurfaceKHR() const { return handle; }
	};

	class device {
	private:
		VkDevice handle;
	public:
		device(vk::physical_device& physical_device, vk::surface& surface);
		~device();
		operator VkDevice() const { return handle; }
		uint32_t queue_family_index;
	};

	class queue {
	private:
		VkQueue handle;
	public:
		queue(vk::device& device, uint32_t queue_family_index);
		operator VkQueue() const { return handle; }
	};

	class swapchain {
	private:
		VkSwapchainKHR handle;
		vk::device& m_device;
	public:
		swapchain(vk::device& device, vk::surface& surface, uint32_t width, uint32_t height);
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
		~shader_module();
		operator VkShaderModule() const { return handle; }
	};
}
