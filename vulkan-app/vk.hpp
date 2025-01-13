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
}
