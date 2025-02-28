

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vulkan/vulkan.h>

#include "gfx.hpp"
#include "graphics/context.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#define WIDTH 800
#define HEIGHT 800

int main()
{
	app::window window;
	graphics::context context(window.handle, WIDTH, HEIGHT);

    input::init_keyboard(window.glfw_window);
    input::init_mouse(window.glfw_window);

	app::engine gfx(context);

    try
    {
		gfx.update(context, window);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
