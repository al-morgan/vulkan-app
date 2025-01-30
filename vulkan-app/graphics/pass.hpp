#pragma once

#include <functional>
#include <vector>


#include "vulkan/vulkan.h"

#include "graphics/buffer.hpp"
#include "graphics/image.hpp"
#include "graphics/descriptor_set.hpp"

namespace graphics
{
	class pass
	{
	private:
		VkPipeline m_pipeline;
		std::vector<std::reference_wrapper<graphics::buffer>> m_buffers;
		std::vector<std::reference_wrapper<graphics::image>> m_images;

		std:: vector<graphics::descriptor_set> descriptor_sets;

		// What I need:
		// Descriptor sets
		// Transitions
	};
}
