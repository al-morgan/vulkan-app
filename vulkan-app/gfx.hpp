#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace app
{
	class gfx
	{
	private:
		GLFWwindow* m_window;
		VkInstance m_instance;
		VkPhysicalDevice m_physical_device;
		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkQueue m_graphics_queue;
		VkQueue m_present_queue;
		uint32_t m_queue_family_index;	// Used for both graphics and present for now.
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchain_images;
		std::vector<VkImageView> m_swapchain_image_views;

		void set_up_glfw();
		void tear_down_glfw();
		void set_up_instance();
		void tear_down_instance();
		void pick_physical_device();
		void set_up_surface();
		void tear_down_surface();
		void set_up_device();
		void tear_down_device();
		void set_up_queues();
		void tear_down_queues();
		void set_up_swap_chain();
		void tear_down_swap_chain();

	public:
		gfx();
		~gfx();
		void update();
	};
}
