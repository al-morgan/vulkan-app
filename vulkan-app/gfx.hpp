#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace app
{
	class gfx
	{
	private:
		GLFWwindow* m_window;

	public:
		gfx();
		static void init_instance();
		void init_window();
	};
}
