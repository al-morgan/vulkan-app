#pragma once

#include <stdexcept>
#include <glm/glm.hpp>

namespace graphics
{
	struct vertex2d
	{
		glm::vec2 pos;
	};

	struct vertex3d
	{
		glm::vec3 pos;
	};

	struct framebuffer
	{
		VkImage image;
		VkImageView view;
		uint32_t index;
	};

	static void check(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan error!");
		}
	}

}