#include <functional>
#include <vector>


#include "vulkan/vulkan.h"

#include "graphics/buffer.hpp"
#include "graphics/image.hpp"

namespace graphics
{
	class pass
	{
	private:
		VkPipeline m_pipeline;
		std::vector<std::reference_wrapper<graphics::buffer>> m_buffers;
		std::vector<std::reference_wrapper<graphics::image>> m_images;

		// What I need:
		// Descriptor sets
		// Transitions
	};
}
