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

		void set_up_glfw();
		void tear_down_glfw();
		void set_up_instance();
		void tear_down_instance();
		void pick_physical_device();
		void set_up_device();
		void tear_down_device();


	public:
		gfx();
		~gfx();
		void update();
	};
}
