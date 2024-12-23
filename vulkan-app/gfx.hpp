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
