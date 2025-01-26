#include <vulkan/vulkan.h>
#include "graphics/context.hpp"

namespace graphics
{
	/// <summary>
	/// Handles transfering buffer data to the GPU
	/// </summary>
	class device_image
	{
	private:
		const graphics::context& m_context;
		VkImage m_image;
		VkDeviceMemory m_memory;
	public:
		device_image(const graphics::context& context, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
		device_image() = delete;
		~device_image();

		/// <summary>
		/// Returns the device-local buffer handle.
		/// </summary>
		/// <returns>The buffer handle.</returns>
		VkImage handle();
	};
}
