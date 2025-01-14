#pragma once

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
}
