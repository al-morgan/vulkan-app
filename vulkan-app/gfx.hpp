#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace app
{
	class gfx
	{
	private:
		GLFWwindow* m_window;
		VkInstance m_instance;

		void set_up_glfw();
		void tear_down_glfw();
		void set_up_instance();
		void tear_down_instance();


	public:
		gfx();
		~gfx();
		void update();
	};
}
