#pragma once

#include <windows.h>
#include <vulkan/vulkan.h>

namespace vk
{
	class instance {
	public:
		instance();
		~instance();
		VkInstance handle;
	};

	class physical_device {
	public:
		physical_device(vk::instance& instance);
		VkPhysicalDevice handle;
	};

	class surface {
	private:
		VkInstance instance_handle;
	public:
		surface(vk::instance& instance, HWND window_handle);
		~surface();
		VkSurfaceKHR handle;
	};
}
