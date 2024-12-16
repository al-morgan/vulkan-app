#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace app
{
	class gfx
	{
	private:
		GLFWwindow* m_window;
		void set_up_glfw();
		void tear_down_glfw();

	public:
		gfx();
		~gfx();
		void update();
	};
}
